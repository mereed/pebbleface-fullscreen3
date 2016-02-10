var mConfig = {};

Pebble.addEventListener("ready", function(e) {
	//console.log("taller is ready");
  loadLocalData();
  returnConfigToPebble();
});

Pebble.addEventListener("showConfiguration", function(e) {
	Pebble.openURL(mConfig.configureUrl);
});

Pebble.addEventListener("webviewclosed",
  function(e) {
    if (e.response) {
      var config = JSON.parse(e.response);
      saveLocalData(config);
      returnConfigToPebble();
    }
  }
);

function saveLocalData(config) {

  //console.log("loadLocalData() " + JSON.stringify(config));

  localStorage.setItem("invert", parseInt(config.invert)); 
  localStorage.setItem("bluetoothvibe", parseInt(config.bluetoothvibe)); 
  localStorage.setItem("hourlyvibe", parseInt(config.hourlyvibe)); 
  localStorage.setItem("datesep", parseInt(config.datesep)); 
  localStorage.setItem("hour_col1", parseInt(config.hour_col1)); 
  localStorage.setItem("min_col1", parseInt(config.min_col1)); 
  localStorage.setItem("hour_col2", parseInt(config.hour_col2)); 
  localStorage.setItem("min_col2", parseInt(config.min_col2)); 
  localStorage.setItem("health", parseInt(config.health)); 
  
  loadLocalData();

}
function loadLocalData() {
  
	mConfig.invert = parseInt(localStorage.getItem("invert"));
	mConfig.bluetoothvibe = parseInt(localStorage.getItem("bluetoothvibe"));
	mConfig.hourlyvibe = parseInt(localStorage.getItem("hourlyvibe"));
	mConfig.datesep = parseInt(localStorage.getItem("datesep"));
	mConfig.hour_col1 = parseInt(localStorage.getItem("hour_col1"));
	mConfig.min_col1 = parseInt(localStorage.getItem("min_col1"));
	mConfig.hour_col2 = parseInt(localStorage.getItem("hour_col2"));
	mConfig.min_col2 = parseInt(localStorage.getItem("min_col2"));
	mConfig.health = parseInt(localStorage.getItem("health"));
	mConfig.configureUrl = "http://www.themapman.com/pebblewatch/fullscreen3.html";


	if(isNaN(mConfig.invert)) {
		mConfig.invert = 0;
	}
	if(isNaN(mConfig.bluetoothvibe)) {
		mConfig.bluetoothvibe = 1;
	}
	if(isNaN(mConfig.hourlyvibe)) {
		mConfig.hourlyvibe = 0;
	} 

	if(isNaN(mConfig.datesep)) {
		mConfig.datesep = 0;
	} 
	if(isNaN(mConfig.hour_col1)) {
		mConfig.hour_col1 = 0;
	}
	if(isNaN(mConfig.min_col1)) {
		mConfig.min_col1 = 0;
	}
	if(isNaN(mConfig.hour_col2)) {
		mConfig.hour_col2 = 0;
	}
	if(isNaN(mConfig.min_col2)) {
		mConfig.min_col2 = 0;
	}
	if(isNaN(mConfig.health)) {
		mConfig.health = 0;
	}

  //console.log("loadLocalData() " + JSON.stringify(mConfig));
}
function returnConfigToPebble() {
  //console.log("Configuration window returned: " + JSON.stringify(mConfig));
  Pebble.sendAppMessage({
    "invert":parseInt(mConfig.invert), 
    "bluetoothvibe":parseInt(mConfig.bluetoothvibe), 
    "hourlyvibe":parseInt(mConfig.hourlyvibe),
    "datesep":parseInt(mConfig.datesep),
    "hour_col1":parseInt(mConfig.hour_col1),
    "min_col1":parseInt(mConfig.min_col1),
	"hour_col2":parseInt(mConfig.hour_col2),
    "min_col2":parseInt(mConfig.min_col2),
    "health":parseInt(mConfig.health),
  });    
}