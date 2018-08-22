// Randomly add a data point every 500ms
var random = new TimeSeries();

var time_series = {};


// setInterval(function() {
//   random.append(new Date().getTime(), Math.random() * 10000);
// }, 500);

// function createTimeline() {
//   var chart = new SmoothieChart({millisPerPixel:82});
//   chart.addTimeSeries(random, { strokeStyle: 'rgba(0, 255, 0, 1)', fillStyle: 'rgba(0, 255, 0, 0.2)', lineWidth: 4 });
//   chart.streamTo(document.getElementById("chart"), 1000);
// }

function createChart(time_series){
  var chart = new SmoothieChart({millisPerPixel:82});
  chart.addTimeSeries(time_series, { strokeStyle: 'rgba(0, 255, 0, 1)', fillStyle: 'rgba(0, 255, 0, 0.2)', lineWidth: 4 });
  
  new_chart_element = document.createElement('canvas');
  new_chart_element.style.width='400';
  new_chart_element.style.height='100';

  document.getElementById('chart_containers').appendChild(new_chart_element);
  
  chart.streamTo(new_chart_element, 500);
}


var charts = []


var mqtt_client = new MQTTClientSimplified('lubuntuKafka',9001,'11', ['test']);        

window.addEventListener('msgReceived', function(e){
  // console.log(e.detail.payloadString + e.detail.destinationName);

  var sensor_vals = JSON.parse(JSON.parse(e.detail.payloadString));

  for (source in sensor_vals){
    if (!charts.includes(source)){
      time_series[source] = new TimeSeries();
      createChart(time_series[source]);
      charts.push(source);
    }
    timestamp = new Date().getTime();  /////////////////////////////////////////*

    time_series[source].append(parseInt(timestamp,10), parseFloat(sensor_vals[source]));
  };
});



// var source = new EventSource('http://localhost:5000/topic/test');

// source.onmessage = function(e){
//     // console.log(JSON.parse(e.data))
//     // console.log(parseFloat(e.data))
//     var data = e.data.split('|');
//     var timestamp = data[0];
//     var sensor_vals = JSON.parse(JSON.parse(data[1]));

//     console.log(sensor_vals);

//     for (source in sensor_vals){
//       if (!charts.includes(source)){
//         time_series[source] = new TimeSeries();
//         createChart(time_series[source]);
//         charts.push(source);
//       }
//       console.log(source);
//       time_series[source].append(parseInt(timestamp,10), parseFloat(sensor_vals[source]));
//     }

//     // console.log(parseInt(timestamp,10));
//     console.log(time_series);
// }