var connection = new RsXHRConnection();
var RS = new RsApi(connection);
RS.start();

// implements automatic update using the state token system
// components using this mixin should have a member "path" to specify the resource
var AutoUpdateMixin = 
{
	componentWillMount: function()
	{
		var c = this;
		function on_data_changed()
		{
			RS.request({path: c.path}, response_callback);
		}
		function response_callback(resp)
		{
			//console.log("Mixin received data: "+JSON.stringify(resp));
			var state = c.state;
			state.data = resp.data;
			c.setState(state);
			RS.unregister_token_listener(on_data_changed);
			RS.register_token_listener(on_data_changed, resp.statetoken);
		}
		on_data_changed();
	},
	componentWillUnmount: function()
	{
		RS.unregister_token_listener(this);
	},
};

var Peers = React.createClass({
	mixins: [AutoUpdateMixin],
	path: "peers",
	getInitialState: function(){
		return {data: []};
	},
	render: function(){
		var renderOne = function(f){
			console.log("make one");
			return <p>{f.name} <img src={"../api/v2/peers"+f.locations[0].avatar_address} /></p>;
		};
		return <div>{this.state.data.map(renderOne)}</div>;
	},
 });

var Peers2 = React.createClass({
	mixins: [AutoUpdateMixin],
	path: "peers",
	getInitialState: function(){
		return {data: []};
	},
	render: function(){
		var Peer = React.createClass({
			render: function(){
				var locations = this.props.data.locations.map(function(loc){
					var online;
					if(loc.is_online)
						online = "Online";
					else
						online = "Offline"
					return(<li key={loc.peer_id}>{loc.name} ({online})</li>);
				});
				return(<tr><td>{this.props.data.name}</td><td><ul>{locations}</ul></td></tr>);
			}
		});
		return (
		<table>
			<tr><th> name </th><th> locations</th></tr>
			{this.state.data.map(function(peer){ return <Peer key={peer.name} data={peer}/>; })}
		</table>);
	},
});

React.render(
	<div>
		<ul className="nav">
			<li onClick={function(e){alert("Hi");}}>
				Friends
			</li>
			<li>
				test2
			</li>
		</ul>
		<div>
			<p>the list updates itself when something changes. Lots of magic happens here!</p>
			<Peers2 />
		</div>
	</div>,
	document.body
);