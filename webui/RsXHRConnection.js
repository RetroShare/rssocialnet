/**
 * Connection to the RS backend using XHR
 * (could add other connections later, for example WebSockets)
 * @constructor
 */
function RsXHRConnection()
{
    var debug;
    debug = function(str){console.log(str);};
    //debug = function(str){};

    var server_hostname = "localhost";
    var server_port = "9090";
    var api_root_path = "/api/v2/";

    /**
     * Send a request to the backend
     * automatically encodes the request as JSON before sending it to the server
     * @param {object}  req - the request to send to the server
     * @param {function} cb - callback function to be called to handle the response. The callback takes one object as parameter.
     */
    this.request = function(req, cb)
    {
        //var xhr = window.XMLHttpRequest ? new XMLHttpRequest() : new ActiveXObject("Microsoft.XMLHTTP");
        // TODO: window is not available in QML
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function(){
            //console.log("onreadystatechanged state"+xhr.readyState);
            // TODO: figure out how to catch errors like connection refused
            // maybe want to have to set a state variable like ok=false
            // the gui could then display: "no connection to server"
            if (xhr.readyState === 4) {
                // received response
                debug("RsXHRConnection received response:");
                debug(xhr.responseText);
                cb(JSON.parse(xhr.responseText));
            }
        }
        // post is required for sending data
        var method;
        if(req.data){
            method = "POST";
        } else {
            method = "GET";
        }
        xhr.open(method, "http://"+server_hostname+":"+server_port+api_root_path+req.path);
        var data = JSON.stringify(req.data);
        debug("RsXHRConnection sending data:");
        debug(data);
        xhr.setRequestHeader('Content-Type', 'application/json');
        xhr.send(data);
    };
};