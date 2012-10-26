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
var height = 0;//500 - margin.top - margin.bottom;
var width = 0;
var tension = 0.8;
var connected = false;

connectSlots();
makeHistogram();

function setFrequency(frequency) {
    this.frequency = frequency;
}

function setMeasurement(measurement) {
    this.measurement = measurement;
}

function setData()
{
    dataset = [frequency, measurement];
}

function connectSlots() {
    if (!connected) {
        connected = true;
        try{
            histogramData.sendFrequency.connect(setFrequency);
            histogramData.sendMeasurement.connect(setMeasurement);
            width = histogramData.width - margin.right - margin.left;
            height = histogramData.height - margin.top - margin.left;
            histogramData.emitData();
        }
        catch(e){
            alert(e);
        }
    }
}

function makeHistogram() {
    setData();
    var xScale = d3.scale.linear()
                        .domain([d3.min(dataset, function(d) {
                            return d[1];
                        }), d3.max(dataset, function(d) {
                            return d[1];
                        })])
                        .range([margin.left, width]);

    var yScale = d3.scale.linear()
                        .domain([0, d3.max(dataset, function(d) {
                            return d[0];
                        })])
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

    var line = d3.svg.line()
                    .interpolate("cardinal")
                    .x(function(d) {
                        return xScale(d[1]);
                    })
                    .y(function(d) {
                        return yScale(d[0]);
                    })
                    .tension(tension);

    var area = d3.svg.area()
                    .interpolate("cardinal")
                    .tension(tension)
                    .x(function(d) {
                        return xScale(d[1]);
                    })
                    .y0(height)
                    .y1(function(d) {
                        return yScale(d[0]);
                    });

    svg.append("path")
            .attr("class", "area")
            .attr("fill", "steelblue")
            .attr("opacity", 0.5)
            .attr("d", area(dataset));

    svg.append("path")
            .attr("class", "line")
            .attr("fill", "none")
            .attr("stroke", "black")
            .attr("stroke-width", 1)
            .attr("d", line(dataset));

    // var bar = svg.selectAll("rect")
                    // .data(dataset)
                    // .enter()
                    // .append("rect")
                    // .attr("class", "bar")
                    // .attr("x", function(d, i) {
                        // return xScale(i);
                    // })
                    // .attr("y", function(d) {
                        // return yScale(d);
                    // })
                    // .attr("width", width / (dataset.length + 1))
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
}

