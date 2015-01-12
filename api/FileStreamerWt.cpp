#include "FileStreamerWt.h"

#include <Wt/Http/Response>
#include <Wt/Http/ResponseContinuation>
#include <Wt/Http/Request>
#include <unistd.h>

FileStreamerWt::FileStreamerWt(const RsPlugInInterfaces &ifaces):
    mIfaces(ifaces)
{

}

class FileStreamState
{
public:
    RsFileHash hash;
    uint64_t size;
    uint64_t offset;
};

void FileStreamerWt::handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response)
{
    std::cerr << "FileStreamerWt::handleRequest()" << std::endl;

    Wt::Http::ResponseContinuation* continuation = request.continuation();
    FileStreamState state;
    if (continuation == NULL){
        std::string path = request.pathInfo();

        std::cerr << "FileStreamerWt::handleRequest() not a continuation. path=" << path << std::endl;

        if(path.empty())
        {
            response.setStatus(404);
            response.setMimeType("text/plain");
            response.out() << "Error: no hash given. Append the hash of a file to the current path." << std::endl;
            return;
        }

        // remove the leading slash
        path = path.substr(1);
        RsFileHash hash(path);
        if(hash.isNull())
        {
            response.setStatus(404);
            response.setMimeType("text/plain");
            response.out() << "Error: path is not a valid file hash" << std::endl;
            return;
        }

        FileInfo info;
        std::list<RsFileHash> dls;
        mIfaces.mFiles->FileDownloads(dls);
        if(!(mIfaces.mFiles->alreadyHaveFile(hash, info) || std::find(dls.begin(), dls.end(), hash) != dls.end()))
        {
            response.setStatus(404);
            response.setMimeType("text/plain");
            response.out() << "Error: file not existing on local peer and not downloading. Start the download before streaming it." << std::endl;
            return;
        }
        // init stream to begin
        state.hash = hash;
        state.size = info.size;
        state.offset = 0;
    }
    else
    {
        state = boost::any_cast<FileStreamState>(continuation->data());
        std::cerr << "FileStreamerWt::handleRequest() continuation: hash=" << state.hash.toStdString()
                  << " size=" << state.size << "offset=" << state.offset << std::endl;
    }

    std::vector<uint8_t> data;
    mIfaces.mFiles->getFileData(state.hash, state.offset, data);
    response.out().write((char*)data.data(), data.size());

    std::cerr << "FileStreamerWt::handleRequest() send " << data.size() << "bytes" << std::endl;

    state.offset += data.size();

    if (state.offset != state.size) {
        std::cerr << "FileStreamerWt::handleRequest() creating a continuation" << std::endl;
        continuation = response.createContinuation();
        // remember what to do next
        continuation->setData(state);

        if(data.empty())
        {
            // wait a few milliseconds to make busy waiting for new data not to cpu costly
            usleep(10*1000);
        }
    }
    else
    {
        std::cerr << "FileStreamerWt::handleRequest() reached end of file" << std::endl;
    }

}
