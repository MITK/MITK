document.body.style.backgroundColor = 'rgb(240, 240, 240)';

const minHeight = 255;
var chart;

var chartData;
var errorValuesPlus=[];
var errorValuesMinus=[];
var xValues=[];
var yValues=[];
var dataLabels=[];
var xs = {};

var dataColors = {};
var chartTypes = {};
var lineStyle = {};

var dataProperties = {};

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
        let xDataTemp = channel.objects[propertyName].m_XData;
        let yDataTemp = channel.objects[propertyName].m_YData;
	      let errorsTempPlus = channel.objects[propertyName].m_ErrorDataPlus;
		  let errorsTempMinus = channel.objects[propertyName].m_ErrorDataMinus;
        let dataLabel = channel.objects[propertyName].m_Label;
        dataLabels.push(dataLabel);

        console.log("loading datalabel: "+dataLabel);

        //add label to x array
        xDataTemp.unshift('x'+count.toString())
        xs[dataLabel] = 'x' + count.toString()

        xDataTemp.push(null); //append null value, to make sure the last tick on x-axis is displayed correctly
        yDataTemp.unshift(dataLabel)
        yDataTemp.push(null); //append null value, to make sure the last tick on y-axis is displayed correctly

        xValues[count] = xDataTemp
        yValues[count] = yDataTemp
        errorValuesPlus[count] = errorsTempPlus;
		errorValuesMinus[count] = errorsTempMinus;
		

        let tempLineStyle = '';

        if (channel.objects[propertyName].m_LineStyleName == "solid")
  			{
  			  tempLineStyle = ''
  			}
  			else
  			{
  			  tempLineStyle = { 'style': 'dashed' }
  			}

        dataProperties[dataLabel] = {
            "color" : channel.objects[propertyName].m_Color,
            "chartType": channel.objects[propertyName].m_ChartType,
            "style": tempLineStyle
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
 * Generate error bars object
 *
 * @param {array} errors - contains error bar values
 * @return error bar object
 */
function generateErrorBars(errors, visible){
	let errorObject = {
	  type: 'data',
	  array: errors,
	  visible: visible
	}

	return errorObject;
}

function generateErrorBarsAsymmetric(errorsPlus, errorsMinus, visible){
	let errorObject = generateErrorBars(errorsPlus, visible);
	errorObject["arrayminus"] = errorsMinus;
	errorObject["symmetric"] = false;
	
	return errorObject;
}

function generateLineOptions(options){
  return {
    color : options.color
  }
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

    let inputType = dataProperties[dataLabels[index]]["chartType"];
    let chartType = getPlotlyChartType(inputType);

    let trace = {
      x: xValues[index].slice(1),
      y: yValues[index].slice(1),
      type: chartType,
      name: dataLabels[index]
    };

	  if(typeof errorValuesPlus[index] !== 'undefined'){
		  if(typeof errorValuesMinus[index] !== 'undefined' && errorValuesMinus[index].length > 0)
		  {
			trace["error_y"] = generateErrorBarsAsymmetric(errorValuesPlus[index], errorValuesMinus[index], chartData.m_ShowErrorBars);
		  }else{
			trace["error_y"] = generateErrorBars(errorValuesPlus[index], chartData.m_ShowErrorBars);
		  }
	  }

    if (dataProperties[dataLabels[index]]["style"] == "dashed"){
      trace["line"]["dash"] = "dot"
    }

    // ===================== CHART TYPE OPTIONS HANDLING ===========
    if (chartType == "scatter"){
      trace["line"] = generateLineOptions(dataProperties[dataLabels][index]["color"])
    }

    data.push(trace)
  }

  //=============================== STYLE ========================
  let marginTop = chartData.m_diagramTitle == undefined ? 10 : 50;

  if (chartData.m_LegendPosition == "bottomMiddle"){
	  var legendX = 0.5;
	  var legendY = -0.75;
  }
  else if (chartData.m_LegendPosition == "bottomRight"){
	  var legendX = 1;
	  var legendY = 0;
  }
  else if (chartData.m_LegendPosition == "topRight"){
	  var legendX = 1;
	  var legendY = 1;
  }
  else if (chartData.m_LegendPosition == "topLeft"){
	  var legendX = 0;
	  var legendY = 1;
  }
  else if (chartData.m_LegendPosition == "middleRight"){
	  var legendX = 1;
	  var legendY = 0.5;
  }
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
	showlegend: chartData.m_ShowLegend,
	legend: {
		x: legendX,
		y: legendY
	}
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

function Reload(){
  console.log("Reload chart");
  generateChart(chartData);
}

function SetShowSubchart(showSubchart)
{
  chartData.m_ShowSubchart = showSubchart;
}

function SetStackDataString(stackDataString)
{
  chartData.m_StackedData = stackDataString;
}

function SetShowErrorBars(showErrorBars)
{
  chartData.m_ShowErrorBars = showErrorBars;
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

  dataProperties[dataLabels[0]]["chartType"] = transformTo; // preserve chartType for later updates
  let plotlyType = getPlotlyChartType(transformTo);
  let chart = document.getElementById("chart");
  let update = {type : plotlyType}
  Plotly.restyle(chart, update, 0); // updates the given plotly trace at index 0 with an update object built of a standard trace object
};
