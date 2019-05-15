function addGraph(names, tabName){
	$(document).ready(function() {
		var series = [];
		for (i in names) {
			series.push({
				name: names[i],
				data: (function () {
					var data3 = [];
					$.ajax({
						url: 'http://0.0.0.0:5000/data/'+names[i],
						async: false,
						dataType: 'json',
						success: function (json) {   
							assignVariable(json);
						}
					});

					function assignVariable(data) {
						data3 = data;
					}
					console.log(data3);
					return data3;
				}())
			})

		}
		jQuery.noConflict();
		var theme = 'default';
		(function($){ // encapsulate jQuery
			// Create the chart
			Highcharts.stockChart(tabName, {
				chart: {
					events: {
						load: function () {
							// set up the updating of the chart each second
							var series = this.series;
							setInterval(function () {
								for (i in names) {

									var url = 'http://0.0.0.0:5000/lastMeas/'+names[i];
									var data2 = [];

									$.ajax({
										url: url,
										async: false,
										dataType: 'json',
										success: function (json) {   
											assignVariable(json);
										}
									});

									function assignVariable(data) {
										data2 = data;
									}
										
									var x = data2[0], 
									y = data2[1];

									console.log(series[i].data.length);
									var last_index = series[i].data.length;
									if (typeof x !== 'undefined') {
										if(series[i].data[last_index-1].x !== x){
											if(series[i].data.length < 1000){
												series[i].addPoint([x, y]);
											}
											else{
												series[i].addPoint([x, y], true, true);
											}
										}
									}
								}
							}, 1000);
						}
					}
				},

				time: {
					useUTC: false
				},

				plotOptions: {
					line: {
						animation: false
					}
				},

				xAxis: {
					type: 'datarate',
					dateTimeLabelFormats: { // don't display the dummy year
						month: '%e. %b',
						year: '%b'
					},
					title: {
						text: 'Time'
					},
					ordinal: false
				},


				rangeSelector: {
					buttons: [{
						count: 1,
						type: 'minute',
						text: '1M'
					}, {
						count: 5,
						type: 'minute',
						text: '5M'
					}, {
						type: 'all',
						text: 'All'
					}],
					inputEnabled: false,
					selected: 0
				},

				title: {
					text: 'data'
				},

				exporting: {
					enabled: false
				},

				series: series

			});
		})(jQuery);


	});

}

