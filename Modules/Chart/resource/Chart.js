document.body.style.backgroundColor = 'rgb(240, 240, 240)';

const minHeight = 255;
var chart;

var xErrorValuesPlus=[];
var xErrorValuesMinus=[];
var yErrorValuesPlus=[];
var yErrorValuesMinus=[];
var xValues=[];
var yValues=[];
var dataLabels=[];
var pieDataLabels=[];
var xs = {};

var minValueX;
var maxValueX;
var minValueY;
var maxValueY;
var dataColors = {};
var chartTypes = {};
var lineStyle = {};
var backgroundColor = '#f0f0f0';
var foregroundColor = 'black';

var dataProperties = {};

var chartData = null;


// Important loading function. This will be executed at first in this whole script.
// Fetching data from QWebChannel and storing them for display purposes.
window.onload = function()
{
  initHeight();
  
	 /**
	 * Adds handler for Qt signal emitted from QmitkChartxyData.
	 * Changes for single traces like value changes in a line plot are handled here.
	 */
	function handleDataChangeEvents(registeredChannelObject)
	{
	  let position = registeredChannelObject.m_LabelCount;
	  registeredChannelObject.SignalDiagramTypeChanged.connect(function(newValue){
		let updateDiagramType = generateTraceByChartType(newValue);
		Plotly.restyle('chart', updateDiagramType, position);
	  });

	  registeredChannelObject.SignalLineStyleChanged.connect(function(newValue){
		var dashValue;
		if (newValue == "dashed"){
			dashValue = "dot";
		}
		else {
			dashValue = "solid";
		}
		updateDash = {
				line : {
					"dash" : dashValue
				}
			}

		Plotly.restyle('chart', updateDash, position);
	  });

	  registeredChannelObject.SignalColorChanged.connect(function(newValue){
		var updateColor={
			marker:{
				"color" : newValue
			},
			line:{
				"color" : newValue
			}
		}
		Plotly.restyle('chart', updateColor, position);
	  });

	  registeredChannelObject.SignalDataChanged.connect(function(newValue){
		console.log("data changed for label " + registeredChannelObject.m_Label);

		let xDataTemp = registeredChannelObject.m_XData;
		let yDataTemp = registeredChannelObject.m_YData;

		let trace = generateTraceByChartType(registeredChannelObject.m_ChartType);

		trace["x"] = [xDataTemp];
		trace["y"] = [yDataTemp];
		trace["name"] = registeredChannelObject.m_Label;

		Plotly.restyle('chart', trace, position);
	  });

	  registeredChannelObject.SignalLabelChanged.connect(function(newValue){
		let trace = {
			  name: newValue
			};
		Plotly.restyle('chart', trace, position);
	  });
	}

	/**
	 * Adds handler for Qt signal emitted from QmitkChartData.
	 * Changes for the whole chart like title are handled here.
	 */
	function handleChartChangeEvents(registeredChannelObject)
	{
	  registeredChannelObject.SignalXAxisLabelChanged.connect(function(newValue){
		var layout = {
		  xaxis: {
			title: {
			text: newValue
			},
			color: foregroundColor
		  }
		}
		Plotly.relayout('chart', layout);
	  });

	  registeredChannelObject.SignalYAxisLabelChanged.connect(function(newValue){
		var layout = {
		  yaxis: {
			title: {
			text: newValue
			},
			color: foregroundColor
		  }
		}
		Plotly.relayout('chart', layout);
	  });

	  registeredChannelObject.SignalTitleChanged.connect(function(newValue){
		var layout = {
		      title: {
				text:newValue,
				font: {
					color: foregroundColor
				}
			  }
		}
		Plotly.relayout('chart', layout);
	  });

	  registeredChannelObject.SignalLegendPositionChanged.connect(function(newValue){
		let legendPosition = generateLegendPosition(chartData.m_LegendPosition);
		var layout = {
				legend: legendPosition
		}
		Plotly.relayout('chart', layout);
	  });

	  registeredChannelObject.SignalYAxisScaleChanged.connect(function(newValue){
		var layout = {
			yaxis : {
				type : newValue
			}
		};
		Plotly.relayout('chart', layout);
	  });

	  registeredChannelObject.SignalShowLegendChanged.connect(function(newValue){
		var layout = {
			showlegend : newValue
		};
		Plotly.relayout('chart', layout);
	  });

	  registeredChannelObject.SignalShowSubchartChanged.connect(function(newValue){
		var layout = {
			xaxis : {}
		};
		if (newValue){
			layout.xaxis.rangeslider = {}; // adds range slider below x axis
		}
		Plotly.relayout('chart', layout);
	  });

	}

  new QWebChannel(qt.webChannelTransport, function(channel) {
    chartData = channel.objects.chartData;

	handleChartChangeEvents(chartData);
	
    let count = 0;
    for(let propertyName in channel.objects) {
      if (propertyName != 'chartData')
      {
		let chartXYData = channel.objects[propertyName];
		handleDataChangeEvents(chartXYData);

        let xDataTemp = chartXYData.m_XData;
        let yDataTemp = chartXYData.m_YData;
        let xErrorsTempPlus = chartXYData.m_XErrorDataPlus;
		let pieDataLabelsTemp = chartXYData.m_PieLabels;
        let xErrorsTempMinus = chartXYData.m_XErrorDataMinus;
        let yErrorsTempPlus = chartXYData.m_YErrorDataPlus;
        let yErrorsTempMinus = chartXYData.m_YErrorDataMinus;
        let dataLabel = chartXYData.m_Label;
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
		pieDataLabels[count] = pieDataLabelsTemp;


        var tempLineStyle = '';

        if (chartXYData.m_LineStyleName == "solid")
        {
          tempLineStyle = ''
        }
        else
        {
          tempLineStyle = "dashed"
        }

        dataProperties[dataLabel] = {
            "color" : chartXYData.m_Color,
            "chartType": chartXYData.m_ChartType,
            "style": tempLineStyle
        }

        count++;
      }
    }
	var theme = chartData.m_themeName;
	minValueX = chartData.m_MinValueXView;
	minValueY = chartData.m_MinValueYView;
	maxValueX = chartData.m_MaxValueXView;
	maxValueY = chartData.m_MaxValueYView;

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

/**
 * Generate legend position  object
 *
 * @param legendPosition the string of the legendPosition enum from QmitkChartWidget
 * @return legend position object
 */
function generateLegendPosition(legendPosition){
  if (legendPosition == "bottomMiddle"){
	  var legendX = 0.5;
	  var legendY = -0.75;
  }
  else if (legendPosition == "bottomRight"){
	  var legendX = 1;
	  var legendY = 0;
  }
  else if (legendPosition == "topRight"){
	  var legendX = 1;
	  var legendY = 1;
  }
  else if (legendPosition == "topLeft"){
	  var legendX = 0;
	  var legendY = 1;
  }
  else if (legendPosition == "middleRight"){
	  var legendX = 1;
	  var legendY = 0.5;
  }

	let legendPositionObject = {
		x: legendX,
		y: legendY,
		font : {
			color: foregroundColor
		}
	}

	return legendPositionObject;
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

function generateTraceByChartType(chartType){
	let plotlyChartType = getPlotlyChartType(chartType);

	let trace = {
      type: plotlyChartType,
    };
	trace["line"] = {}
	if (plotlyChartType == "area"){
      trace["fill"] = 'tozeroy'
    } else if (plotlyChartType == "spline"){
      trace["line"]["shape"] = 'spline'
    } else if (plotlyChartType == "scatterOnly"){
      trace["mode"] = 'markers';
    } else if (plotlyChartType == "area-spline"){
      trace["fill"] = 'tozeroy'
      trace["line"]["shape"] = 'spline'
    }

	return trace;
}

function generatePlotData(){
  let data = [];

  for (let index = 0; index < dataLabels.length; index++){
	let trace = generateTraceByChartType(dataProperties[dataLabels[index]]["chartType"]);

	trace["x"] = xValues[index].slice(1);
	trace["y"] = yValues[index].slice(1);
	trace["name"] = dataLabels[index];
	if (dataProperties[dataLabels[index]]["chartType"]=="pie"){
		trace["values"] = yValues[index].slice(1);
		if (typeof pieDataLabels[index] !== 'undefined' && pieDataLabels[index].length > 0){
		  trace["labels"] = pieDataLabels[index];
		}
	}

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
	trace["line"]["color"] = dataProperties[dataLabels[index]]["color"]

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

  let legendPosition = generateLegendPosition(chartData.m_LegendPosition);

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
	legend: legendPosition
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

    Plotly.plot('chart', data, layout, {displayModeBar: false, responsive: true});

if (minValueX !== null && maxValueX !== null){
  UpdateMinMaxValueXView(minValueX, maxValueX);
}
  if (minValueY !== null && maxValueY !== null){
  UpdateMinMaxValueYView(minValueY, maxValueY);
}
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
