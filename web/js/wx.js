function loadWeather(loc) {
    $.ajax({
        url: "http://api.wunderground.com/api/92550999fb0db4b9/conditions/q/"+loc+".json",
    	dataType: "jsonp",
    	success: parsewx
    });
}

// parse wunderground data
function parsewx(json) {
    wx = json.current_observation;
	document.getElementById('ftr_out').innerHTML = wx.temp_f+"&deg; F";
}