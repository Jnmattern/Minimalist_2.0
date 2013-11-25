var seconds = 0;
var displaymode = 0;

function logVariables() {
	console.log("	seconds: " + seconds);
	console.log("	displaymode: " + displaymode);
}

Pebble.addEventListener("ready", function() {
	console.log("Ready Event");
	
	seconds = localStorage.getItem("seconds");
	if (!seconds) {
		seconds = 1; // Default: show seconds
	}

	displaymode = localStorage.getItem("displaymode");
	if (!displaymode) {
		displaymode = 0; // Default: show Hours & Minutes
	}
	
	logVariables();
						
	Pebble.sendAppMessage(JSON.parse('{"seconds":'+seconds+',"displaymode":'+displaymode+'}'));

});

Pebble.addEventListener("showConfiguration", function(e) {
	console.log("showConfiguration Event");

	logVariables();
						
	Pebble.openURL("http://www.famillemattern.com/jnm/pebble/Minimalist/Minimalist_2.0.1.php?" +
				   "seconds=" + seconds +
				   "&displaymode=" + displaymode );
});

Pebble.addEventListener("webviewclosed", function(e) {
	console.log("Configuration window closed");
	console.log(e.type);
	console.log(e.response);

	var configuration = JSON.parse(e.response);
	Pebble.sendAppMessage(configuration);
	
	seconds = configuration["seconds"];
	localStorage.setItem("seconds", seconds);
	
	displaymode = configuration["displaymode"];
	localStorage.setItem("displaymode", displaymode);
});
