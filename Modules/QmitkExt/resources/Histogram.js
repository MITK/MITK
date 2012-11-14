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
    top : 30,
    bottom : 0,
    left : 50,
    right : -20,
    };
var height = histogramData.height - margin.top - margin.bottom;
var width = histogramData.width - margin.left - margin.right;
var tension = 0.8;
var connected = false;
   /*  var line = d3.svg.line()
                    .interpolate("cardinal")
                    .x(function(d,i) {
                        return xScale(histogramData.measurement[i]);
                    })
                    .y(function(d,i) {
                        return yScale(d);
                    })
                    .tension(tension);

    var area = d3.svg.area()
                    .interpolate("cardinal")
                    .tension(tension)
                    .x(function(d,i) {
                        return xScale(histogramData.measurement[i]);
                    })
                    .y0(height)
                    .y1(function(d,i) {
                        return yScale(d);
                    }); */

if (!connected)
    {
        connected = true;
        histogramData.DataChanged.connect(changeHistogram);
        //makeHistogram();
    }



    var xScale = d3.scale.linear()
                        .domain([d3.min(histogramData.measurement), d3.max(histogramData.measurement)])
                        .range([margin.left, width]);

    var yScale = d3.scale.linear()
                        .domain([0, d3.max(histogramData.frequency)])
                        .range([height, margin.top]);

    var xAxis = d3.svg.axis()
                    .scale(xScale)
                    .orient("bottom");

    var yAxis = d3.svg.axis()
                    .scale(yScale)
                    .orient("left");

    var svg = d3.select("body")
                .append("svg")
                .attr("class", "svg")
                .attr("width", width + margin.left + margin.right)
                .attr("height", height + margin.bottom + margin.top)
                .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

    // svg.append("path")
            // .attr("class", "area")
            // .attr("fill", "steelblue")
            // .attr("opacity", 0.5)
            // .attr("d", area(histogramData.frequency));

    // svg.append("path")
            // .attr("class", "line")
            // .attr("fill", "none")
            // .attr("stroke", "black")
            // .attr("stroke-width", 1)
            // .attr("d", line(histogramData.frequency));

    // var bar = svg.selectAll("rect")
                    // .data(histogramData.frequency)
                    // .enter()
                    // .append("rect")
                    // .attr("class", "bar")
                    // .attr("x", function(d,i) {
                        // return xScale(histogramData.measurement[i]);
                    // })
                    // .attr("y", function(d,i) {
                        // return yScale(d);
                    // })
                    // .attr("width", 1)
                    // .attr("height", function(d) {
                        // return height - yScale(d);
                    // })
                    // .attr("fill", "grey")
                    // .attr("opacity", 0.5);

    svg.append("g")
            .attr("class", "axis")
            .attr("transform", "translate(" + 0 + "," + height + ")")
            .call(xAxis);

    svg.append("g")
            .attr("class", "axis")
            .attr("transform", "translate(" + margin.left + "," + 0 + ")")
            .call(yAxis);


function changeHistogram ()
{
    var bar = svg.selectAll("rect")
                .data(data);

    bar.data(histogramData.frequency).enter().append("rect")
        .attr("class", "bar")
        .attr("x", function(d,i) {
            return xScale(histogramData.measurement[i]);
        })
        .attr("y", height)
        .attr("height", 0)
        .attr("width", 1)
        .attr("fill", "grey")
        .transition()
        .attr("height", function(d) {
            return (height-yScale(d));
        })
        .attr("width", 1)
        .attr("y", function(d) {
            return yScale(d);
        });

    bar.transition()
        .attr("x", function(d,i) {
            return xScale(histogramData.measurement[i]);
        })
        .attr("y", function(d) {
            return yScale(d);
        })
        .attr("height", function(d) {
            return (height-yScale(d));
        })
        .attr("width", 1)
        .attr("fill", "grey");

    bar.exit().transition()
        .attr("y", height)
        .attr("height", 0)
        .remove();
}

