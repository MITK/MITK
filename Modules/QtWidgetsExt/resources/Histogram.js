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
var min;
var max;

/*
 * Connecting signals from qt side with JavaScript methods.
 */
if (!connected)
{
  connected = true;
  histogramData.SignalDataChanged.connect(updateHistogram);
  histogramData.SignalGraphChanged.connect(updateHistogram);
}

function disconnectSignals()
{
  histogramData.SignalDataChanged.disconnect(updateHistogram);
  histogramData.SignalGraphChanged.disconnect(updateHistogram);
  delete histogramData;
}

/*
 * Predefinition of scales.
 */
var xScale = d3.scale.linear()
  .domain([d3.min(histogramData.measurement)-binSize/2,d3.max(histogramData.measurement)+binSize/2])
  .range([0,width]);

var yScale = d3.scale.linear()
  .domain([d3.min(histogramData.frequency),d3.max(histogramData.frequency)])
  .range([height,margin.top]);

/*
 * Predefinition of axis elements.
 */
var xAxis = d3.svg.axis()
  .scale(xScale)
  .orient("bottom")
  .tickFormat(d3.format("s"));

var yAxis = d3.svg.axis()
  .scale(yScale)
  .orient("left")
  .tickFormat(d3.format("s"));

/*
 * Predefinition of the zoom.
 */
var zoombie = d3.behavior.zoom().x(xScale).scaleExtent([1, 50]).on("zoom", zoom);

/*
 * Creation of the svg element, which holds the complete histogram.
 */
var svg = d3.select("body")
  .append("svg")
  .attr("class", "svg")
  .attr("width", width + margin.right + margin.left)
  .attr("height", height + margin.top + margin.bottom)
 .append("g")
  .attr("transform", "translate (" + margin.left + "," + margin.top + ")")
  .call(zoombie)
  .on("mousemove", myMouseMove);

/*
 * Appending a rectangle to the svg, to guarantee the possibility
 * of zooming on the whole histogram.
 */
svg.append("rect")
  .attr("width", width + margin.left + margin.right)
  .attr("height", height + margin.top + margin.bottom)
  .attr("opacity", 0);

/*
 * Appending a second svg to main svg, which holds only the graph.
 */
var vis = svg.append("svg")
  .attr("width", width)
  .attr("height", height);

/*
 * Predefinition of the lines.
 */
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

/*
 * Method to update the histogram data
 * and to change the displayed graph.
 */
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

/*
 * Calculation of the bin size.
 */
function calcBinSize()
{
  if (1 < histogramData.measurement.length)
  {
    min = d3.min(histogramData.measurement);
    max = d3.max(histogramData.measurement);
    binSize = ((max - min) / (histogramData.measurement.length-1));
  }
  else
  {
    binSize = 10;
  }
}

/*
 * Method to display histogram as a barchart.
 */
function barChart()
{
  definition();

  /*
   * Change zoom to a fixed y-axis.
   */
  zoombie = d3.behavior.zoom().x(xScale).scaleExtent([1, 50]).on("zoom", zoom);

  svg.call(zoombie);

  /*
   * Element to animate transition from linegraph to barchart.
   */
  vis.selectAll("path.line").remove();
  vis.selectAll("circle").remove();

  /*
   * Definition of the bar elements.
   */
  var bar = vis.selectAll("rect.bar").data(histogramData.frequency);

  /*
   * Definition how to handle new bar elements.
   */
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

  /*
   * Definition how to handle changed bar elements.
   */
  bar.transition()
    .duration(dur)
    .attr("x", function(d,i) {
      return xScale(histogramData.measurement[i]-binSize/2);
    })
    .attr("y", myYPostion)
    .attr("height", barHeight)
    .attr("width", barWidth);

  /*
   * Definition how to handle bar elements which doesn't exist anymore.'
   */
  bar.exit()
    .transition()
    .duration(dur)
    .attr("y", height)
    .attr("height", 0)
    .remove();

  /*
   * Update of axis elements.
   * First delete old ones, then generate new.
   */
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

/*
 * Method to display histogram as a linegraph.
 */
function linePlot()
{
  definition();

  /*
   * Change zoom to a zoomable y-axis.
   */
  zoombie = d3.behavior.zoom().x(xScale).y(yScale).scaleExtent([1, 50]).on("zoom", zoom);

  svg.call(zoombie);
  /*
   * Elements to animate transitions from barchart to linegraph.
   * Different transition when an intensity profile is generated.
   */
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
      .attr("y", height) // <--
      .attr("height", 0)
      .remove();
  }

  /*
   * Creating circle elements, when an intensity profile is generated to show tooltips.
   * Due performance losses tooltips are not supported for line histograms.
   */
  if(histogramData.intensityProfile)
  {
  var circles = vis.selectAll("circle").data(histogramData.frequency);

  /*
   * Definition how to handle new circle elements.
   */
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

  /*
   * Definition how to handle bar elements which doesn't exist anymore.
   */
  circles.exit().remove();
  }
  else
  {
    /*
     * Removing of all circle elements if a line histogram is generated.
     */
    vis.selectAll("circle").remove();
  }

  /*
   * Creating a new path element.
   */
  var graph = vis.selectAll("path.line")
    .data([histogramData.frequency]);

/*
 * Definition how to handle a new path element, using predefined lines.
 */
  graph.enter()
    .append("path")
    .attr("class", "line")
    .transition()
    .duration(dur)
    .attr("d", line);

/*
 * Definition how to handle change points in an existing path element.
 */
  graph.transition()
    .duration(dur)
    .attr("d", line);

  /*
   * Update of axis elements.
   * First delete old ones, then generate new.
   */
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
/*
 * Match scale to current data.
 */
  xScale = d3.scale.linear()
    .domain([d3.min(histogramData.measurement)-binSize/2,d3.max(histogramData.measurement)+binSize/2])
    .range([0,width]);

  yScale = d3.scale.linear()
    .domain([d3.min(histogramData.frequency),d3.max(histogramData.frequency)])
    .range([height,margin.top]);

/*
 * Match axes to current scale
 */
  xAxis = d3.svg.axis()
    .scale(xScale)
    .orient("bottom")
    .tickFormat(d3.format("s"));


  yAxis = d3.svg.axis()
    .scale(yScale)
    .orient("left")
    .tickFormat(d3.format("s"));
}

/*
 * Method to calculate barwidth in px.
 */
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
  /*
   * Ensure barwidth is not smaller than 1px.
   */
  bw = bw > 1 ? bw : 1;
  return bw;
}

/*
 * Method to calculate barheight in px.
 * Ensure barheight is not smaller than 1px.
 */
function barHeight(d)
{
  var bh;
  bh = height - yScale(d);
  bh = bh >=2 ? bh : 2;
  return bh;
}

/*
 * Method to calculate dynamical y positions.
 */
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

/*
 * Method to fit parameters of bars/line when zoomed.
 * Update axes elements.
 * Resets the view if scale is 1.
 */
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

/*
 * Method to show infobox, while mouse is over a bin.
 */
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
  if ((min >= 0) && (max <= 2)) //tooltip for float images
  {
    d3.select(".measurement").text("Greayvalue: " + (Math.round(x*1000)/1000));
  }
  else
  {
    d3.select(".measurement").text("Greyvalue: " + (Math.round(x*10)/10) + " ... " + (Math.round((x+binSize)*10)/10));
  }
  d3.select(".frequency").text("Frequency: " + y);
}

/*
 * Hide infobox, when mouse not over a bin.
 */
function myMouseOut()
{
  var myBar = d3.select(this);
  myBar.style("fill", d3.rgb(0,71,185));
  d3.select(".infobox").style("display", "none");
}

/*
 * Show tooltip, while mouse is over a circle in an intensity profile.
 */
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
  d3.select(".frequency").text("Intensity: " + y);
}

/*
 * Hide infobox, when mouse not over a circle.
 */
function myMouseOutLine()
{
  var myCircle = d3.select(this);
  myCircle.attr("opacity", 0);
  d3.select(".infobox").style("display", "none");
}

/*
 * Update mousecoordinates when mouse is moved.
 * Tooltip is shown on the right side of the mouse until it reaches the right boundary,
 * the it switches to the left side.
 */
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
