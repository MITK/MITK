/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

var margin = {
  top : 10,
  bottom : 50,
  left : 45,
  right : 20,
  };
var height = histogramData.height - margin.top - margin.bottom;
var width = histogramData.width - margin.left - margin.right;
var tension = 0.8;
var connected = false;
var dur = 1000;
var binSize = 10;

// connecting signal from qt side with JavaScript method
if (!connected)
{
  connected = true;
  histogramData.DataChanged.connect(updateHistogram);
  histogramData.GraphChanged.connect(updateHistogram);
}

var xScale = d3.scale.linear()
  .domain([d3.min(histogramData.measurement)-binSize/2,d3.max(histogramData.measurement)+binSize/2])
  .range([0,width]);

var yScale = d3.scale.linear()
  .domain([d3.min(histogramData.frequency),d3.max(histogramData.frequency)])
  .range([height,margin.top]);

var xAxis = d3.svg.axis()
  .scale(xScale)
  .orient("bottom")
  .tickFormat(d3.format("s"));

var yAxis = d3.svg.axis()
  .scale(yScale)
  .orient("left")
  .tickFormat(d3.format("s"));

var zoombie = d3.behavior.zoom().x(xScale).scaleExtent([1, 50]).on("zoom", zoom);

var svg = d3.select("body")
  .append("svg")
  .attr("class", "svg")
  .attr("width", width + margin.right + margin.left)
  .attr("height", height + margin.top + margin.bottom)
 .append("g")
  .attr("transform", "translate (" + margin.left + "," + margin.top + ")")
  .call(zoombie)
  .on("mousemove", myMouseMove);

svg.append("rect")
  .attr("width", width + margin.left + margin.right)
  .attr("height", height + margin.top + margin.bottom)
  .attr("opacity", 0);

var vis = svg.append("svg")
  .attr("width", width)
  .attr("height", height);

var line = d3.svg.line()
  .interpolate("linear")
  .x(function(d,i) {
    return xScale(histogramData.measurement[i]-binSize/2);
  })
  .y(function(d) {
    return yScale(d);
  });

var linenull = d3.svg.line()
  .interpolate("linear")
  .x(function(d,i) {
    return xScale(histogramData.measurement[i]-binSize/2);
  })
  .y(function(d) {
    return yScale(0);
  });

updateHistogram();

// method to update and choose histogram
function updateHistogram()
{
  calcBinSize();
  if (!histogramData.useLineGraph)
  {
    barChart();
  }
  else if (histogramData.useLineGraph)
  {
    linePlot()
  }
}

function calcBinSize()
{
  var min = d3.min(histogramData.measurement);
  var max = d3.max(histogramData.measurement);
  binSize = ((max - min) / (histogramData.measurement.length));
}

// method to display histogram as barchart
function barChart()
{
  definition();
  zoombie = d3.behavior.zoom().x(xScale).scaleExtent([1, 50]).on("zoom", zoom);

  svg.call(zoombie);

// element to animate transition from linegraph to barchart
  vis.selectAll("path.line").remove();
  vis.selectAll("circle").remove();

  var bar = vis.selectAll("rect.bar").data(histogramData.frequency);

  bar.enter().append("rect")
    .attr("class", "bar")
    .on("mouseover", myMouseOver)
    .on("mouseout", myMouseOut)
    .attr("x", function(d,i) {
      return xScale(histogramData.measurement[i]-binSize/2);
    })
    .attr("y", height)
    .attr("height", 0)
    .attr("width", barWidth)

  bar.transition()
    .duration(dur)
    .attr("x", function(d,i) {
      return xScale(histogramData.measurement[i]-binSize/2);
    })
    .attr("y", myYPostion)
    .attr("height", barHeight)
    .attr("width", barWidth);

  bar.exit()
    .transition()
    .duration(dur)
    .attr("y", height)
    .attr("height", 0)
    .remove();

  svg.selectAll("g")
    .remove();

  svg.append("g")
    .attr("class", "x axis")
    .attr("transform", "translate(0," + height + ")")
    .call(xAxis);

  svg.append("g")
    .attr("class", "y axis")
    .call(yAxis);
}

// method to display histogram as linegraph
function linePlot()
{
  definition();
  zoombie = d3.behavior.zoom().x(xScale).y(yScale).scaleExtent([1, 50]).on("zoom", zoom);

  svg.call(zoombie);
// element to animate transition from barchart to linegraph
  if(!histogramData.intensityProfile)
  {
    vis.selectAll("rect.bar")
      .transition()
      .duration(dur)
      .attr("height", 0)
      .remove();
  }
  else
  {
    vis.selectAll("rect.bar")
      .transition()
      .duration(dur)
      .attr("y", height)
      .attr("height", 0)
      .remove();
  }

  if(histogramData.intensityProfile)
  {
  var circles = vis.selectAll("circle").data(histogramData.frequency);

  circles.enter()
    .append("circle")
    .on("mouseover", myMouseOverLine)
    .on("mouseout", myMouseOutLine)
    .attr("cx", function(d,i) {
      return xScale(histogramData.measurement[i]-binSize/2);
    })
    .attr("cy", function (d) {
      return yScale(d)
    })
    .attr("r", 5)
    .attr("opacity", 0)
    .style("stroke", "red")
    .style("stroke-width", 1)
    .style("fill-opacity", 0);

  circles.exit().remove();
  }
  else
  {
    vis.selectAll("circle").remove();
  }

  var graph = vis.selectAll("path.line")
    .data([histogramData.frequency]);

  graph.enter()
    .append("path")
    .attr("class", "line")
    .transition()
    .duration(dur)
    .attr("d", line);

  graph.transition()
    .duration(dur)
    .attr("d", line);

  svg.selectAll("g")
    .remove();

  svg.append("g")
    .attr("class", "x axis")
    .attr("transform", "translate(0," + height + ")")
    .call(xAxis);

  svg.append("g")
    .attr("class", "y axis")
    .call(yAxis);
}

function definition()
{
// match scale to current data
  xScale = d3.scale.linear()
    .domain([d3.min(histogramData.measurement)-binSize/2,d3.max(histogramData.measurement)+binSize/2])
    .range([0,width]);

  yScale = d3.scale.linear()
    .domain([d3.min(histogramData.frequency),d3.max(histogramData.frequency)])
    .range([height,margin.top]);

  xAxis = d3.svg.axis()
    .scale(xScale)
    .orient("bottom")
    .tickFormat(d3.format("s"));

  yAxis = d3.svg.axis()
    .scale(yScale)
    .orient("left")
    .tickFormat(d3.format("s"));
}

// method to ensure barwidth is not smaller than 1px
function barWidth(d, i)
{
  var bw;
  if (i != (histogramData.measurement.length-1))
  {
    bw =(xScale(histogramData.measurement[i + 1]) - xScale(histogramData.measurement[i])) * (histogramData.frequency.length / (histogramData.frequency.length + 1)) - 1;
  }
  else
  {
    bw =(xScale(histogramData.measurement[i]) - xScale(histogramData.measurement[i - 1])) * (histogramData.frequency.length / (histogramData.frequency.length + 1)) - 1;
  }
  bw = bw > 1 ? bw : 1;
  return bw;
}

function barHeight(d)
{
  var bh;
  bh = height - yScale(d);
  bh = bh >=2 ? bh : 2;
  return bh;
}

function myYPostion(d)
{
  var myy = yScale(d);
  myy = (height-myy) > 2 ? myy : (height-2);
  if (d == 0)
  {
    return height;
  }
  return myy;
}

// zoom function, with plot focus by scale 1 and different zooming mode
function zoom()
{
  if (zoombie.scale() == 1)
  {
    zoombie.translate([0,0]);
    xScale.domain([d3.min(histogramData.measurement)-binSize/2,d3.max(histogramData.measurement)+binSize/2]);
    yScale.domain([d3.min(histogramData.frequency),d3.max(histogramData.frequency)]);
  }
  if (!histogramData.useLineGraph)
  {
    svg.select(".x.axis").call(xAxis);
    vis.selectAll(".bar")
      .attr("width", barWidth)
      .attr("x", function(d, i) {
        return xScale(histogramData.measurement[i]-binSize/2);
        });
  }
  else
  {
    svg.select(".x.axis").call(xAxis);
    svg.select(".y.axis").call(yAxis);
    vis.selectAll("path.line")
      .attr("transform", "translate(" + zoombie.translate() + ")scale(" + zoombie.scale() + ")")
      .style("stroke-width", 1 / zoombie.scale());
    vis.selectAll("circle")
      .attr("cx", function(d, i) {
        return xScale(histogramData.measurement[i]-binSize/2);
      })
      .attr("cy", function(d) {
        return yScale(d);
      });
  }
}

// method to show infobox, while mouse is over a bin
function myMouseOver()
{
  var myBar = d3.select(this);
  var reScale = d3.scale.linear()
    .domain(xScale.range())
    .range(xScale.domain());
  var y = myBar.data();
  var x = reScale(myBar.attr("x"));

  myBar.style("fill", "red");
  d3.select(".infobox").style("display", "block");
  d3.select(".measurement").text("Greyvalue: " + (Math.round(x)) + " ... " + (Math.round(x+binSize)));
  d3.select(".frequency").text("Frequency: " + y);
}

// hide infobox, when mouse not over a bin
function myMouseOut()
{
  var myBar = d3.select(this);
  myBar.style("fill", d3.rgb(0,71,185));
  d3.select(".infobox").style("display", "none");
}

function myMouseOverLine()
{
  var myCircle = d3.select(this)
  var reScale = d3.scale.linear()
    .domain(xScale.range())
    .range(xScale.domain());
  var y = myCircle.data();
  var x = reScale(myCircle.attr("cx"));

  x = x >= 0 ? x : 0;

  myCircle.attr("opacity", 1);
  d3.select(".infobox").style("display", "block");
  d3.select(".measurement").text("Distance: " + (Math.round(x*100)/100) + " mm");
  d3.select(".frequency").text("Intesity: " + y);
}

function myMouseOutLine()
{
  var myCircle = d3.select(this);
  myCircle.attr("opacity", 0);
  d3.select(".infobox").style("display", "none");
}

// update mousecoordinates by mousemove
function myMouseMove()
{
  var infobox = d3.select(".infobox");
  var coords = d3.mouse(this);
  if ((coords[0]+120)<(width-margin.right))
  {
    infobox.style("left", coords[0] + 75 + "px");
    infobox.style("top", coords[1] + "px");
  }
  else
  {
    infobox.style("left", coords[0] - 90 + "px");
    infobox.style("top",coords[1] + "px");
  }
}
