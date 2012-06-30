function loadChannels(index) {
	var d = new Date();
	var yr = d.getFullYear();
	var day = pad(d.getDate());
	var mnth = pad(d.getMonth()+1);
	var hour = pad(d.getHours());
	var min = pad(d.getMinutes());
	var sec = pad(d.getSeconds());
	var date = yr+''+mnth+''+day+''+hour+''+min+''+sec;
	$.ajax({
		url: 'http://www22.verizon.com/xml/schedule?regionId=91628&dateTime='+date+'&duration=60&startIndex='+index+'&channelCount=2000&src=wireless',
		type: 'GET',
		dataType: ($.browser.msie) ? "text" : "xml",
		error: function(XMLHttpRequest, textStatus, errorThrown,data){
			alert("Error: data: "+data+" Status: "+textStatus+" Error: "+errorThrown);
		},
		success: parseChannels 
	});
};

function parseChannels(xml) {
	xml = $(xml.responseText); //pull XML from JSON. fucking IE.
	$(xml).find("meta").each(function() {
		//save the next index number we need.
		var idx = $(this).attr("endIndex");
	});
	
	$(xml).find("chl").each(function() {
		//grab basic channel information.
		var channelNme = $(this).attr("name");
		var channelNum = $(this).attr("num");
		var channelSgn = $(this).attr("sgn");
		//grab what's on the channel...
		chRow = '<tr><td><p>'+channelNum+' <img src=\"img/chls/half/'+channelNum+'.png\"</img>'+channelSgn+'</p></td>';
		$(this).find("schdl").each(function() {
			var shoFlgs 	= $(this).attr("flgs");
			var shoStart 	= $(this).attr("start");
			var shoRtl		= $(this).attr("rtl"); //some kind of short title?
			var shoTitle	= $(this).attr("title"); //Long titles.
			var shoEpName	= $(this).attr("epi_title");
			var shoFiOSID	= $(this).attr("fiosId");
			var shoType		= $(this).attr("type");
			chRow += '<td><a href="#" onClick="loadShowInfo('+shoFiOSID+','+channelNum+','+shoStart+');">'+shoTitle+'</a> ('+shoType+')</td>';
		});
		chRow += '</tr>';
		$('#chlist').append(chRow);
	});
	$('#tvguide').tableScroll();
	// $('#tvguide').tableScroll({width:470}); 
};

function loadShowInfo(shoFiOSID,channelNum,shoStart) {
	$.ajax({
		url: 'http://www22.verizon.com/xml/detail?regionId=91628&gzip=false&fiosId='+shoFiOSID+'&channelNum='+channelNum+'&dateTime='+shoStart+'&src=wireless',
		type: 'GET',
		dataType: ($.browser.msie) ? "text" : "xml",
		error: function(XMLHttpRequest, textStatus, errorThrown,data){
			alert("Error: data: "+data+" Status: "+textStatus+" Error: "+errorThrown);
		},
		success: parseShowInfo 
	});
};	


function parseShowInfo(xml){
	shoXML = $(xml.responseText); //pull the xml from json.
	var program = $(shoXML).find("program");

	var year    = $(program).find("year").text();
	var rating  = $(program).find("mpaa_rating").text();
	var stars   = $(program).find("star_rating").text();
	var length  = $(program).find("run_time").text();
	var lang    = $(program).find("language").text();
	var ctype   = $(program).find("content_type").text();
	var stype   = $(program).find("show_type").text();
	var title   = $(program).find("title").text();
	var desc    = $(program).find("description").text();
	var advis   = $(program).find("advisory_description").text();
	var genre   = $(program).find("genre").text();

	//run through cast.
	$(program).find("cast").each(function() {
		var fn = $(this).find("first_name").text();
		var ln = $(this).find("last_name").text();
		var ro = $(this).find("role").text();
	});

	//run through showings.
	$(program).find("schedule").each(function() {
		var repeat	= $(this).attr("repeat");
		var adate	= $(this).find("air_date").text();
		var atime	= $(this).find("air_time").text();
		var durat	= $(this).find("duration").text();
		$(this).find("station").each(function() {
			var chnum = $(this).attr("channel_num");
			var call = $(this).find("station_call_sign").text();
			var station = $(this).find("station_name").text();
		});
	});
};

function chk_scroll(e) {
	console.log("scroll...")
    var elem = $(e.currentTarget);
    if (elem[0].scrollHeight - elem.scrollTop() == elem.outerHeight()) {
        alert("bottom");
    }
}

function pad(number) {
	return (number < 10 ? '0' : '') + number
}