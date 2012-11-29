/**
 * @author Moritz Petry
 */
//var dataset = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 7, 18, 19, 22];

/*var indexNumber = Math.round(Math.random()*10+30);
 for (var i = 0; i< indexNumber; i++) {
 var newNumber = Math.random() * 100;
 dataset.push(newNumber);
 }*/

var dataset = new Array();
var frequency = new Array();
var measurement = new Array();
var dataset;

var margin = {
  top : 0,
  bottom : 50,
  left : 45,
  right : 20,
  };
var height = histogramData.height - margin.top - margin.bottom;
var width = histogramData.width - margin.left - margin.right;
var tension = 0.8;
var connected = false;
var dur = 1000;
var useLinePlot = false;

// connecting signal from qt side with JavaScript method
if (!connected)
{
  connected = true;
  histogramData.DataChanged.connect(updateHistogram);
}

var xScale = d3.scale.linear()
  .domain([d3.min(histogramData.measurement),d3.max(histogramData.measurement)])
  .range([0,width]);

var yScale = d3.scale.linear()
  .domain([0,d3.max(histogramData.frequency)])
  .range([height,margin.top]);

var xAxis = d3.svg.axis()
  .scale(xScale)
  .orient("bottom");

var yAxis = d3.svg.axis()
  .scale(yScale)
  .orient("left");

var svg = d3.select("body")
  .append("svg")
  .attr("class", "svg")
  .attr("width", width + margin.right + margin.left)
  .attr("height", height + margin.top + margin.bottom)
 .append("g")
  .attr("transform", "translate (" + margin.left + "," + margin.top + ")")
  .call(d3.behavior.zoom().x(xScale).y(yScale).scaleExtent([1, 10]).on("zoom", zoom));

var vis = svg.append("svg")
  .attr("width", width)
  .attr("height", height);

svg.append("rect")
  .attr("width", width + margin.left + margin.right)
  .attr("height", height + margin.top + margin.bottom)
  .attr("opacity", 0);

updateHistogram();

// method to update and choose histogram
function updateHistogram()
{
  if (!histogramData.useLineGraph)
  {
    barChart();
  }
  else if (histogramData.useLineGraph)
  {
    linePlot()
  }
}

// method to display histogram as barchart
function barChart()
{
// match scale to current data
  xScale = d3.scale.linear()
    .domain([d3.min(histogramData.measurement),d3.max(histogramData.measurement)])
    .range([0,width]);

  yScale = d3.scale.linear()
    .domain([0,d3.max(histogramData.frequency)])
    .range([height,margin.top]);

  xAxis = d3.svg.axis()
    .scale(xScale)
    .ticks(5)
    .orient("bottom");

  yAxis = d3.svg.axis()
    .scale(yScale)
    .orient("left");

  svg.call(d3.behavior.zoom().x(xScale).y(yScale).scaleExtent([1, 10]).on("zoom", zoom));

  var linenull = d3.svg.line()
    .interpolate("linear")
    .x(function(d,i) {
      return xScale(histogramData.measurement[i]);
    })
    .y(function(d) {
      return yScale(0);
    });

// element to animate transition from linegraph to barchart
  vis.selectAll("path.line").transition().duration(dur).attr("d", linenull(histogramData.frequency)).remove();

  var bar = vis.selectAll("rect.bar").data(histogramData.frequency);

  bar.enter().append("rect")
    .attr("class", "bar")
    .on("mouseover", function (d) {d3.select(this).style('fill', "red");})
    .on("mouseout", function (d) {d3.select(this).style("fill", "steelblue");})
    .attr("x", function(d,i) {
      return xScale(histogramData.measurement[i]);
    })
    .attr("y", height)
    .attr("height", 0)
    .attr("width", barWidth)
    .transition().delay(dur).duration(dur*1.5)
    .attr("height", function(d) {
      return (height - yScale(d));
    })
    .attr("width", barWidth)
    .attr("y", function(d) {
      return yScale(d);
    });

  bar.transition().delay(dur).duration(dur*1.5)
    .attr("x", function(d,i) {
      return xScale(histogramData.measurement[i]);
    })
    .attr("y", function(d) {
      return yScale(d);
    })
    .attr("height", function(d) {
      return (height - yScale(d));
    })
    .attr("width", barWidth)

  bar.exit().transition().delay(dur).duration(dur*1.5)
    .attr("y", height)
    .attr("height", 0)
    .remove();

  svg.selectAll("g")
    .transition()
    .duration(dur)
    .attr("opacity", 0)
    .remove();

  svg.append("g")
    .attr("class", "x axis")
    .attr("transform", "translate(0," + height + ")")
    .transition().duration(dur)
    .attr("opacity", 100)
    .call(xAxis);

  svg.append("g")
    .attr("class", "y axis")
    .transition().duration(dur)
    .attr("opacity", 100)
    .call(yAxis);
}

// method to display histogram as linegraph
function linePlot()
{
// match scale to current data
  xScale = d3.scale.linear()
    .domain([d3.min(histogramData.measurement),d3.max(histogramData.measurement)])
    .range([0,width]);

  yScale = d3.scale.linear()
    .domain([0,d3.max(histogramData.frequency)])
    .range([height,margin.top]);

  xAxis = d3.svg.axis()
    .scale(xScale)
    .ticks(5)
    .orient("bottom");

  yAxis = d3.svg.axis()
    .scale(yScale)
    .orient("left");

  svg.call(d3.behavior.zoom().x(xScale).y(yScale).scaleExtent([1, 10]).on("zoom", zoom));

// element to animate transition from barchart to linegraph
  vis.selectAll("rect.bar")
    .transition()
    .duration(dur)
    .attr("height", 0)
    .attr("fill", "black")
    .remove();

  var line = d3.svg.line()
    .interpolate("cardinal")
    .x(function(d,i) {
      return xScale(histogramData.measurement[i]);
    })
    .y(function(d) {
      return yScale(d);
    })
    .tension(0.8);

  var linenull = d3.svg.line()
    .interpolate("cardinal")
    .x(function(d,i) {
      return xScale(histogramData.measurement[i]);
    })
    .y(function(d) {
      return yScale(0);
    })
    .tension(0.8);

  var graph = vis.selectAll("path.line").data([histogramData.frequency]);

  graph.enter()
    .append("path")
    .attr("class", "line")
    .attr("d", linenull)
    .transition()
    .duration(dur)
    .attr("d", line);

  graph.transition()
    .duration(dur)
    .attr("d", line);

  graph.exit().transition().duration(dur).attr("d", linenull);

  svg.selectAll("g")
    .transition()
    .duration(dur)
    .attr("opacity", 0)
    .remove();

  svg.append("g")
    .attr("class", "x axis")
    .attr("transform", "translate(0," + height + ")")
    .transition().duration(dur)
    .attr("opacity", 100)
    .call(xAxis);

  svg.append("g")
    .attr("class", "y axis")
    .transition().duration(dur)
    .attr("opacity", 100)
    .call(yAxis);

}

// method to ensure barwidth is not smaller than 1px
function barWidth()
{
  var bw = width/histogramData.frequency.length - 1;
  if (bw < 1)
  {
    bw = 1;
  }
  return bw;
}

function zoom()
{
  svg.select(".x.axis").call(xAxis);
  svg.select(".y.axis").call(yAxis);
  vis.selectAll(".bar").attr("transform", "translate(" + d3.event.translate + ")scale(" + d3.event.scale + ")");
  vis.selectAll("path.line").attr("transform", "translate(" + d3.event.translate + ")scale(" + d3.event.scale + ")");
}

svg.append("rect")
  .attr("width", width + margin.left + margin.right)
  .attr("height", height + margin.top + margin.bottom)
  .attr("opacity", 0);
