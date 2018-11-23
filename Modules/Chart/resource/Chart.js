document.body.style.backgroundColor = 'rgb(240, 240, 240)';

const minHeight = 255;
var chart;

var chartData;
var xValues=[];
var yValues=[];
var dataLabels=[];
var xs = {};
var dataColors = {};
var chartTypes = {};
var lineStyle = {};

// Important loading function. This will be executed at first in this whole script.
// Fetching data from QWebChannel and storing them for display purposes.
window.onload = function()
{
  initHeight();

  new QWebChannel(qt.webChannelTransport, function(channel) {
    chartData = channel.objects.chartData;

    let count = 0;
  	for(let propertyName in channel.objects) {
  	  if (propertyName != 'chartData')
  	  {
  			let xDataTemp = channel.objects[propertyName].m_XData
  			let yDataTemp = channel.objects[propertyName].m_YData
  			let dataLabel = channel.objects[propertyName].m_Label
  			dataLabels.push(dataLabel)

        console.log("loading datalabel: "+dataLabel);

        //add label to x array
  			xDataTemp.unshift('x'+count.toString())
  			xs[dataLabel] = 'x' + count.toString()

  			xDataTemp.push(null); //append null value, to make sure the last tick on x-axis is displayed correctly
  			yDataTemp.unshift(dataLabel)
  			yDataTemp.push(null); //append null value, to make sure the last tick on y-axis is displayed correctly
  			xValues[count] = xDataTemp
  			yValues[count] = yDataTemp
  			dataColors[dataLabel] = channel.objects[propertyName].m_Color
  			chartTypes[dataLabel] = channel.objects[propertyName].m_ChartType

  			if (channel.objects[propertyName].m_LineStyleName == "solid")
  			{
  			  lineStyle[dataLabel] = ''
  			}
  			else
  			{
  			  lineStyle[dataLabel] = [{ 'style': 'dashed' }]
  			}

  			count++;
  		}
  	}

    generateChart(chartData);

  });
}

/**
 * Inits the height of the chart element to 90% of the full window height.
 */
function initHeight() {
  var size = window.innerHeight-(window.innerHeight/100*10); //subtract 10% of height to hide vertical scrool bar
  let chart = document.getElementById("chart");
  chart.style.height = `${size}px`;
}

function getPlotlyChartType(inputType){
  let plotlyType = "";
  if (inputType == "line"){
    plotlyType = "scatter";
  } else if (inputType == "bar"){
    plotlyType = "bar";
  }
  return plotlyType;
}



/**
 * Here, the chart magic takes place. Plot.ly is called.
 *
 * @param {object} chartData - containing the options for plotting, not the actual values
 */
function generateChart(chartData)
{
  console.log("generate chart");
	if (chartData == undefined)
	{
		chartData = {}
	}

	if (dataLabels == undefined)
	{
    dataLabels = []
	}

  //=============================== DATA ========================
  let data = [];

  for (let index = 0; index < dataLabels.length; index++){

    let inputType = chartTypes[dataLabels[index]];
    let chartType = getPlotlyChartType(inputType);

    let trace = {
      x: xValues[index].slice(1),
      y: yValues[index].slice(1),
      type: chartType,
      name: dataLabels[index],
    };

    if (lineStyle[dataLabels[index]]["style"] == "dashed"){
      trace["line"]["dash"] = "dot"
    }

    data.push(trace)
  }

  //=============================== STYLE ========================
  let marginTop = chartData.m_diagramTitle == undefined ? 10 : 50;
  var layout = {
    title: chartData.m_diagramTitle,
    xaxis: {
      title: chartData.m_xAxisLabel
    },
    yaxis: {
      title: chartData.m_yAxisLabel
    },
    margin: {
      l: 50,
      r: 10,
      b: 50,
      t: marginTop,
      pad: 4
    },
  };

  if (chartData.m_YAxisScale){
      layout.yaxis["type"] = "log"
  }

  if (chartData.m_ShowSubchart){
    layout.xaxis.rangeslider = {}; // adds range slider below x axis
  }

  Plotly.newPlot('chart', data, layout, {displayModeBar: false, responsive: true});
}

/**
 * Change theme of chart.
 *
 * @param {string} color - dark or not dark
 */
function changeTheme(color) {
link = document.getElementsByTagName("link")[0];
  if (color == 'dark') {
    link.href = "Chart_dark.css";
  }
  else
  {
    link.href = "Chart.css";
  }
};

/**
 * Reload the chart with the given arguments.
 *
 * This method is called by C++. Changes on signature with caution.
 * @param {boolean} showSubchart
 * @param {string} stackDataString
 */
function ReloadChart(showSubchart, stackDataString)
{
    chartData.m_ShowSubchart = showSubchart;
    chartData.m_StackedData = stackDataString;
    generateChart(chartData);
}

/**
 * Transforms the view to another chart type.
 *
 * This method is called by C++. Changes on signature with caution.
 * @param {string} transformTo - 'line' or 'bar'
 */
function transformView(transformTo) {
  console.log("transform view");
  console.log(transformTo);

  let plotlyType = getPlotlyChartType(transformTo);
  let chart = document.getElementById("chart");
  let update = {type : plotlyType}
  Plotly.restyle(chart, update, 0); // updates the given plotly trace at index 0 with an update object built of a standard trace object
};
