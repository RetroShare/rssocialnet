#include "imageresize.h"
#include <cstring>
#include <iostream>
#include "jpgd.h"
#include "jpge.h"
#include "lodepng.h"

namespace ImageUtil{

class ImageView{
public:
    ImageView(uint32_t width, uint32_t height, uint8_t components, uint8_t* data):
        mWidth(width), mHeight(height), mComponents(components), mData(data), dummyPixel(123)
    {}

    // get a reference ot a pixel value
    // this functions checks image boundaries
    uint8_t& getPixelRef(uint32_t x, uint32_t y, uint8_t component);

private:
    uint32_t mWidth;
    uint32_t mHeight;
    uint8_t mComponents;
    uint8_t* mData;
    uint8_t dummyPixel;
};

uint8_t& ImageView::getPixelRef(uint32_t x, uint32_t y, uint8_t component)
{
    // image width and height has to be at least one pixel.
    // not sure if a image without pixels can exist in png/jpeg. if yes this case should be handled
    if((mWidth == 0) || (mHeight == 0) || (mComponents == 0))
    {
        std::cerr << "ImageView::getPixelRef() ERROR: image does not have any pixels." << std::cerr;
        return dummyPixel;
    }

    if(mData == 0)
    {
        std::cerr << "ImageView::getPixelRef() ERROR: mData is NULL." << std::cerr;
        return dummyPixel;
    }

    // limit to valid values
    // nice: uints can't be negative, so have to check only one bound
    if(x >= mWidth)
    {
        x = mWidth - 1;
    }
    if(y >= mHeight)
    {
        y = mHeight - 1;
    }
    if(component >= mComponents)
    {
        component = mComponents - 1;
    }
    return mData[(((mWidth*y)+x)*mComponents)+component];
}

enum ImageFormat{ UNKNOWN, JPEG, PNG };

// problems:
// - can't upscale images
// - can't downscale images when one side is to small
// - to slow
// - trouble with png with alpha channel
void limitImageSize(const std::vector<uint8_t>& in, std::vector<uint8_t>& out, uint32_t dstwidth, uint32_t dstheight)
{
    ImageFormat imageFormat = UNKNOWN;
    uint32_t srcwidth;
    uint32_t srcheight;
    int components = 3;
    uint8_t* srcImageBuff = NULL;

    if(in.size() > 3 )
    {
        // check if jpeg
        // (jpeg magic number)
        if((in[0] == 0xFF)&&(in[1] == 0xD8)&&(in[2] == 0xFF))
        {
            imageFormat = JPEG;
            int w;
            int h;
            srcImageBuff = jpgd::decompress_jpeg_image_from_memory(in.data(), in.size(), &w, &h, &components, components);
            if(srcImageBuff == NULL)
            {
                std::cerr << "limitImageSize() error: could not uncompress jpg image data" << std::endl;
                out.clear();
                return;
            }
            srcwidth = w;
            srcheight = h;
        }
        // check if png
        // 89 50 4e 47  (png magic number)
        else if((in[0] == 0x89)&&(in[1] == 0x50)&&(in[2] == 0x4e)&&(in[3] == 0x47))
        {
            imageFormat = PNG;
            int code = lodepng_decode24(&srcImageBuff, &srcwidth, &srcheight, in.data(), in.size());
            if(code != 0)
            {
                std::cerr << "limitImageSize() error: could not uncompress png image data" << std::endl;
                out.clear();
                return;
            }
        }
    }
    if(imageFormat == UNKNOWN)
    {
        std::cerr << "limitImageSize() error: unsupported image format" << std::endl;
        out.clear();
        return;
    }

    // current algorithm can only handle downscaling
    // todo: still resize when one direction is larger than allowed
    if((srcheight <= dstheight) || (srcheight <= dstheight))
    {
        if(imageFormat == JPEG)
        {
            free(srcImageBuff);
        }
        std::cerr << "limitImageSize() error: can't upscale images, won't modify this image" << std::endl;
        out = in;
        return;
    }

    uint8_t* tmpImageBuf = new uint8_t[components*srcwidth*srcheight];
    uint8_t* dstImageBuf = new uint8_t[components*dstwidth*dstheight];

    ImageView srcImage(srcwidth, srcheight, components, srcImageBuff);
    ImageView tempImage(srcwidth, srcheight, components, tmpImageBuf);
    ImageView dstImage(dstwidth, dstheight, components, dstImageBuf);
    // blur the source image
    int xratio = srcwidth / dstwidth;
    int yratio = srcheight / dstheight;
    // make an even number to not darken the image because of the missing fraction when dividing by two
    if(xratio&0x01){ xratio++; }
    if(yratio&0x01){ yratio++; }
    for(int component = 0; component < components; component++)
    {
        for(int x = 0; x<srcwidth; x++)
        {
            for(int y = 0; y<srcheight; y++)
            {
                // sum up all pixels of a region
                double newval = 0;
                for(int px = (x-xratio/2); px<(x+xratio/2); px++)
                {
                    for(int py = (y-yratio/2); py<(y+yratio/2); py++)
                    {
                        newval += srcImage.getPixelRef(px, py, component);
                    }
                }
                tempImage.getPixelRef(x, y, component) = newval / (xratio*yratio);
            }
        }
    }
    // pick some pixels from the blurred image and copy to dstimage
    // could optimize this to only calculate the wanted pixels
    for(int component = 0; component < components; component++)
    {
        for(int x = 0; x<dstwidth; x++)
        {
            for(int y = 0; y<dstheight; y++)
            {
                dstImage.getPixelRef(x, y, component) = tempImage.getPixelRef(x * xratio, y * yratio, component);
            }
        }
    }

    // compress as jpg
    // assuming jpeg will not increase the data size
    // so the size of the raw image shuld be enough
    int estimatedJpgSize = components*dstwidth*dstheight;
    // jpge wants at least 1024 bytes
    if(estimatedJpgSize < 1024)
    {
        estimatedJpgSize = 1024;
    }
    uint8_t* jpgBuff = new uint8_t[estimatedJpgSize];
    bool ok = jpge::compress_image_to_jpeg_file_in_memory(jpgBuff, estimatedJpgSize, dstwidth, dstheight, components, dstImageBuf);
    if(ok)
    {
        out.resize(estimatedJpgSize);
        memcpy(out.data(), jpgBuff, estimatedJpgSize);
    }
    else
    {
        std::cerr << "limitImageSize() error: could not compress as jpg" << std::endl;
    }
    delete jpgBuff;

    free(srcImageBuff);
    delete tmpImageBuf;
    delete dstImageBuf;
}

} // namespace ImageUtil
