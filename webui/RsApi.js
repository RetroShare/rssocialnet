/**
 * JS Api for Retroshare
 * @constructor
 * @param {object} connection - an object which implements a request() function.
 * The request function should take two parameters: an object to be send as request and a callback.
 * The callback should get called with an response object on success.
 */
function RsApi(connection)
{
    var runnign = true;
    /**
     * Send a request to the server
     * @param req - the request so send
     * @param {Function} cb - callback function which takes the response as parameter
     */
    this.request = function(req, cb)
    {
        connection.request(req, cb);
    };
    var tokenlisteners = [];
    /**
     * Register a callback to be called when the state token expired.
     * @param {Function} listener - the callback function, which does not take arguments
     * @param token - the state token to listen for
     */
    this.register_token_listener = function(listener, token)
    {
        tokenlisteners.push({listener:listener, token:token});
    };
    /**
     * Unregister a previously registered callback.
     */
    this.unregister_token_listener = function(listener) // no token as parameter, assuming unregister from all listening tokens
    {
        for(var i=0; i<tokenlisteners.length; i++){
            if(tokenlisteners[i].listener === listener){
                // copy the last element to the current index
                tokenlisteners[i] = tokenlisteners[tokenlisteners.length-1];
                // remove LAST element
                tokenlisteners.pop();
            }
        }
    };
    /**
     * start polling for state changes
     */
    this.start = function(){
        running = true;
        setTimeout(tick, TICK_INTERVAL);
    }
    /**
     * stop polling for state changes
     */
    this.stop = function(){
        running = false;
    }

    // ************** interal stuff **************
    var TICK_INTERVAL = 3000;
    function received_tokenstates(resp)
    {
        if(resp.data){
            for(var i=0; i<resp.data.length; i++){
                var token = resp.data[i];
                // search the listener for this token
                for(var j=0; j<tokenlisteners.length; j++){
                    if(tokenlisteners[i].token === token){
                        // call the listener
                        tokenlisteners[i].listener();
                    }
                }
            }
        }
        // schedule new update
        if(running)
            setTimeout(tick, TICK_INTERVAL);
    };
    function tick()
    {
        var data = [];
        // maybe cache the token list?
        // profiler will tell us if we should
        for(var i=0; i<tokenlisteners.length; i++){
            data.push(tokenlisteners[i].token);
        }
        connection.request({
            path: "statetokenservice",
            data: data,
        }, received_tokenstates);
    };
};

// with this trick, we should be able to run in browser or nodejs
if(typeof window === 'undefined')
{
    // we are running in nodejs, so have to add to export
    module.exports = RsApi;
}