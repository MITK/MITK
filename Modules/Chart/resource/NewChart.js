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

window.onload = function()
{
  initWidth();

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

		    console.log(chartTypes[dataLabel]);

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

    GenerateChart(chartData);

  });
}

function initWidth() {
  var size = window.innerHeight-(window.innerHeight/100*10); //subtract 10% of height to hide vertical scrool bar

  console.log("minHeight:");
  console.log(size);

  let chart = document.getElementById("chart");
  chart.style.height = `${size}px`;
  console.log(chart.style.height);
}

//Here, the chart magic takes place. Plot.ly is called
function GenerateChart(chartData)
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

	var groupLabels = [];
	if (chartData.m_StackedData == true)
	{
    groupLabels = dataLabels
  }

  xValues[0] = xValues[0].slice(1);
  yValues[0] = yValues[0].slice(1);


  let trace1 = {
    x: xValues[0],
    y: yValues[0],
    type: 'bar',
    name: dataLabels[0]
  };

  //===============================
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

  let data = [trace1];

  Plotly.newPlot('chart', data, layout, {displayModeBar: false, responsive: true});
}

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

function ReloadChart(showSubchart, stackDataString)
{
    chartData.m_ShowSubchart = showSubchart;
    chartData.m_StackedData = stackDataString;

    GenerateChart(chartData);
}

function transformView(TransformTo) {
  console.log("transform view");
  console.log(TransformTo);

  let plotlyType = "";
  if (TransformTo == "line"){
    plotlyType = "scatter";
  } else if (TransformTo == "bar"){
    plotlyType = "bar";
  }

  let chart = document.getElementById("chart");
  let update = {type : plotlyType}
  Plotly.restyle(chart, update, 0); // updates the given plotly trace at index 0 with an update object built of a standard trace object
};
