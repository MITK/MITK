document.body.style.backgroundColor = 'rgb(240, 240, 240)';

const minHeight = 255;
var chart;

var chartData;
var xErrorValuesPlus=[];
var xErrorValuesMinus=[];
var yErrorValuesPlus=[];
var yErrorValuesMinus=[];
var xValues=[];
var yValues=[];
var dataLabels=[];
var xs = {};

var dataColors = {};
var chartTypes = {};
var lineStyle = {};
var backgroundColor = '#f0f0f0';
var foregroundColor = 'black';

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
        let xErrorsTempPlus = channel.objects[propertyName].m_XErrorDataPlus;
        let xErrorsTempMinus = channel.objects[propertyName].m_XErrorDataMinus;
        let yErrorsTempPlus = channel.objects[propertyName].m_YErrorDataPlus;
        let yErrorsTempMinus = channel.objects[propertyName].m_YErrorDataMinus;  
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
        xErrorValuesPlus[count] = xErrorsTempPlus;
        xErrorValuesMinus[count] = xErrorsTempMinus;
        yErrorValuesPlus[count] = yErrorsTempPlus;
        yErrorValuesMinus[count] = yErrorsTempMinus;
    

        var tempLineStyle = '';

        if (channel.objects[propertyName].m_LineStyleName == "solid")
        {
          tempLineStyle = ''
        }
        else
        {
          tempLineStyle = "dashed"
        }

        dataProperties[dataLabel] = {
            "color" : channel.objects[propertyName].m_Color,
            "chartType": channel.objects[propertyName].m_ChartType,
            "style": tempLineStyle
        }

        count++;
      }
    }
	var theme = chartData.m_themeName;
	setThemeColors(theme);
    generateChart(chartData);
  });
}

/**
 * Inits the height of the chart element to 90% of the full window height.
 */
function initHeight() {
  var size = window.innerHeight-(window.innerHeight/100*5); //subtract 10% of height to hide vertical scrool bar
  let chart = document.getElementById("chart");
  chart.style.height = `${size}px`;
}

function getPlotlyChartType(inputType){
  let plotlyType = inputType;
  if (inputType == "line"){
    plotlyType = "scatter";
  } else if (inputType == "scatter"){
    plotlyType = "scatterOnly"
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

function generateStackPlotData(){
  let data = [];

  for (let index = 0; index < dataLabels.length; index++){
	let inputType = dataProperties[dataLabels[index]]["chartType"];
    let chartType = getPlotlyChartType(inputType);
	
    let trace = {
      x: xValues[index].slice(1),
      y: yValues[index].slice(1),
      stackgroup: 'one',
      name: dataLabels[index],
	  type: chartType,
	  marker:{
		  color: dataProperties[dataLabels[index]]["color"]
	  }
    };

    data.push(trace);
  }
  return data;
}

function generatePlotData(){
  let data = [];

  for (let index = 0; index < dataLabels.length; index++){

    let inputType = dataProperties[dataLabels[index]]["chartType"];
    let chartType = getPlotlyChartType(inputType);
	
    let trace = {
      x: xValues[index].slice(1),
      y: yValues[index].slice(1),
      type: chartType,
      name: dataLabels[index],
    };

	  if(typeof xErrorValuesPlus[index] !== 'undefined'){
		  if(typeof xErrorValuesMinus[index] !== 'undefined' && xErrorValuesMinus[index].length > 0)
		  {
			trace["error_x"] = generateErrorBarsAsymmetric(xErrorValuesPlus[index], xErrorValuesMinus[index], chartData.m_ShowErrorBars);
		  }else{
			trace["error_x"] = generateErrorBars(xErrorValuesPlus[index], chartData.m_ShowErrorBars);
		  }
	  }
	  
	  if(typeof yErrorValuesPlus[index] !== 'undefined'){
		  if(typeof yErrorValuesMinus[index] !== 'undefined' && yErrorValuesMinus[index].length > 0)
		  {
			trace["error_y"] = generateErrorBarsAsymmetric(yErrorValuesPlus[index], yErrorValuesMinus[index], chartData.m_ShowErrorBars);
		  }else{
			trace["error_y"] = generateErrorBars(yErrorValuesPlus[index], chartData.m_ShowErrorBars);
		  }
	  }

    // ===================== CHART TYPE OPTIONS HANDLING ===========
    // initialize line object
    trace["line"] = {}
	
	trace["line"]["color"] = dataProperties[dataLabels[index]]["color"]
    if (chartType == "scatter"){  
    } else if (chartType == "area"){
      trace["fill"] = 'tozeroy'
    } else if (chartType == "spline"){
      trace["line"]["shape"] = 'spline'
    } else if (chartType == "scatterOnly"){
      trace["mode"] = 'markers';
    } else if (chartType == "area-spline"){
      trace["fill"] = 'tozeroy'
      trace["line"]["shape"] = 'spline'
    }

    // handle marker visibility/size/color
    trace["marker"] = {size: chartData.m_DataPointSize, color: dataProperties[dataLabels[index]]["color"]}
    if (chartData.m_DataPointSize == 0){
      trace["mode"] = "lines";
    }

    if (dataProperties[dataLabels[index]]["style"] == "dashed"){
      trace["line"]["dash"] = "dot"
    }

    data.push(trace)
  }
  return data;
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
  var data = [];
  if (chartData.m_StackedData){
    data = generateStackPlotData();
  } else {
    data = generatePlotData();
  }
  //=============================== STYLE ========================
  let marginTop = chartData.m_chartTitle == undefined ? 10 : 50;

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
	  paper_bgcolor : backgroundColor,
	  plot_bgcolor : backgroundColor,
    title: {
		text:chartData.m_chartTitle,
		font: {
		color: foregroundColor
		}
	},
    xaxis: {
      title: {
		text: chartData.m_xAxisLabel
	  },
	  color: foregroundColor
    },
    yaxis: {
      title: {
		text:chartData.m_yAxisLabel
	  },
	  color: foregroundColor
    },
    margin: {
      l: 50,
      r: 10,
      b: 40,
      t: marginTop,
      pad: 4
    },
	showlegend: chartData.m_ShowLegend,
	legend: {
		x: legendX,
		y: legendY,
		font : {
			color: foregroundColor
		}
	}
  };
  
  if (chartData.m_StackedData){
	  layout["barmode"] = 'stack';
  } 
  
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
	setThemeColors(color);
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
 */
function Reload(){
  console.log("Reload chart");
  generateChart(chartData);
}

function SetShowSubchart(showSubchart)
{
    chartData.m_ShowSubchart = showSubchart;
}

function setThemeColors(theme){
	if (theme == 'dark'){
		backgroundColor = '#2d2d30';
		foregroundColor = 'white';
	}
	else {
		backgroundColor = '#f0f0f0';
		foregroundColor = 'black';
	}
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
 * Zooms to the given x-axis min and max values.
 */
function UpdateMinMaxValueXView(minValueX, maxValueX)
{
  //y-Axis can't be adapted for now. See https://github.com/plotly/plotly.js/issues/1876
  let chart = document.getElementById("chart");
  let update = {
	  xaxis:{
		  range:[minValueX, maxValueX]
		  }
	  };
  Plotly.relayout(chart, update);
}

/**
 * Zooms to the given y-axis min and max values.
 */
function UpdateMinMaxValueYView(minValueY, maxValueY)
{
  //x-Axis can't be adapted for now. See https://github.com/plotly/plotly.js/issues/1876
  let chart = document.getElementById("chart");
  let update = {
	  yaxis:{
		  range:[minValueY, maxValueY]
		  }
	  };
  Plotly.relayout(chart, update);
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
  let update = {type : plotlyType};
  Plotly.restyle(chart, update, 0); // updates the given plotly trace at index 0 with an update object built of a standard trace object
};
