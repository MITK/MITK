document.body.style.backgroundColor = 'rgb(240, 240, 240)';

var greyvalue; //needed to display the tooltip in the right format
var binSize = 10;
var min;
var max;

var minHeight = 255;

var chart = c3.generate({
    data: {
        x : 'x', //use first "column" as x axis values
        columns: [],  //initialize empty. Data will be loaded in function setupChart(initValues)
        type: 'bar',
        selection: {
          enabled: false,
          multiple: false,
        }
    },
    legend: {
        position: 'inset'
    },
    grid: {
      y: {
        lines: [{value:0}] //Draws a horizontal line at y=0
      }
    },
    bar: {
        width: {
            ratio: 0.95 // this makes bar width 95% of length between ticks
        }
    },
    zoom: {
        enabled: true,
    },
    subchart: {
        show: true  //Shows a subchart that shows the region the primary chart is zoomed in to by overlay.
    }, 
    axis: {
        x: {
          type: 'category', //only for type 'category' the bars will be rescaled in width on zoom
          tick: {
            multiline: false,
            fit: false, //to make more x labels appear on zoom
            centered: true,
          },
        },
        y: {
          tick: {
            format: d3.format("s"),
          },
          //for some reason, there is an offset for our linegraph. This is prevented by the following lines
          min: 0,
          padding: { top: 0, bottom: 0 }
        }
    },
    //Style data points in linegraph
    point: {
        r: 0.2,
        focus: 
        {
          expand: {
            r: 4
          }
        }
    },
    tooltip: {
      format: {
        title: function (d) {
          var endValue = (parseFloat(greyvalue[d]) + binSize);
          endValue = endValue.toFixed(3);
          return 'Greyvalue: ' + greyvalue[d] + '...' + endValue;
        },
      }
    }
});

var initValues;

window.onload = function() 
{
  new QWebChannel(qt.webChannelTransport, function(channel) {
    initValues = channel.objects.initValues;

    setupChart(initValues)
  });
}

//This is necessary to resize the chart, after the size of the parent changed
window.onresize = function () {
  var size = window.innerHeight-(window.innerHeight/100*10); //subtract 5% of height to hide vertical scrool bar
  
  if (size < minHeight)
  {
    size = minHeight;
  }

  chart.resize({
    height: size,
  });
}

function ReloadChart(useLineChart, showSubchart)
{ 
    initValues.m_UseLineChart = useLineChart;
    initValues.m_ShowSubchart = showSubchart;
    
    var chartType = 'bar';
    if (initValues.m_UseLineChart) {
        chartType = 'line';
    }
  
    if (initValues.m_ShowSubchart) {
        ShowSubchart(chartType)
    }
    else {
        HideSubchart(chartType)
    }
    
    setupChart(initValues);
}

function setupChart(initValues)
{
  window.onresize();

  calcBinSize(initValues);

  //copy measurements to xValues for x-axis-labels and to greyvalues for tooltips
  var xValues = initValues.m_XData.slice(0);
  greyvalue = initValues.m_XData.slice(0);

  for (var i = 0; i < xValues.length; i++)
  {
    greyvalue[i] = greyvalue[i] - (binSize / 2);
    greyvalue[i] = greyvalue[i].toFixed(3);
    //change number format for x axis. Need to do it here, because it is not working on chart generation.
    xValues[i] = xValues[i];
    xValues[i] = xValues[i].toFixed();
    xValues[i] = d3.format("s")(xValues[i]);
  }

  xValues.unshift('x'); //add label to x array
  xValues.push(null); //append null value, to make sure the last tick on x-axis is displayed correctly
  var yValues = initValues.m_YData;
  yValues.unshift('Frequency'); //add label to y array
  xValues.push(null); //append null value, to make sure the last tick on x-axis is displayed correctly
  
  var chartType = 'bar';
    if (initValues.m_UseLineChart) {
        chartType = 'line';
    }

    if (initValues.m_ShowSubchart) {
        ShowSubchart(chartType)
    }
    else {
        HideSubchart(chartType)
    }
    
  chart.unload(); //unload data before loading new data
  chart.load({
        columns:[
          xValues,
          yValues
        ]
  });
}

/*
 * Calculation of the bin size.
 */
function calcBinSize(initValues) {
  if (1 < initValues.m_XData.length) {
    min = d3.min(initValues.m_XData);
    max = d3.max(initValues.m_XData);
    binSize = ((max - min) / (initValues.m_XData.length - 1));
  }
  else {
    binSize = 10;
  }
}

//Transforamtion between bar and line chart
//takes the name of the chart type as a parameter
//calles by QmitkC3jsWidget
function transformView(TransformTo) {
  chart.transform(TransformTo);
};

function changeTheme(color) {
  if (color == 'dark') {
    link = document.getElementsByTagName("link")[0];
    link.href = "Histogram_dark.css";
  }
  else
  {
    link = document.getElementsByTagName("link")[0];
    link.href = "Histogram.css";
  }
};

function ShowSubchart(chartType)
{
  chart = c3.generate({
    data: {
        x : 'x', //use first "column" as x axis values
        columns: [],  //initialize empty. Data will be loaded in function setupChart(initValues)
        type: chartType,
        selection: {
          enabled: false,
          multiple: false,
        }
    },
    legend: {
        position: 'inset'
    },
    grid: {
      y: {
        lines: [{value:0}] //Draws a horizontal line at y=0
      }
    },
    bar: {
        width: {
            ratio: 0.95 // this makes bar width 95% of length between ticks
        }
    },
    zoom: {
        enabled: true,
    },
    subchart: {
        show: true  //Shows a subchart that shows the region the primary chart is zoomed in to by overlay.
    }, 
    axis: {
        x: {
          type: 'category', //only for type 'category' the bars will be rescaled in width on zoom
          tick: {
            multiline: false,
            fit: false, //to make more x labels appear on zoom
            centered: true,
          },
        },
        y: {
          tick: {
            format: d3.format("s"),
          },
          //for some reason, there is an offset for our linegraph. This is prevented by the following lines
          //min: 0,
          //padding: { top: 0, bottom: 0 }
        }
    },
    //Style data points in linegraph
    point: {
        r: 0.2,
        focus: 
        {
          expand: {
            r: 4
          }
        }
    },
    tooltip: {
      format: {
        title: function (d) {
          var endValue = (parseFloat(greyvalue[d]) + binSize);
          endValue = endValue.toFixed(3);
          return 'Greyvalue: ' + greyvalue[d] + '...' + endValue;
        },
      }
    }
  });
}

function HideSubchart(chartType)
{
  chart = c3.generate({
    data: {
        x : 'x', //use first "column" as x axis values
        columns: [],  //initialize empty. Data will be loaded in function setupChart(initValues)
        type: chartType,
        selection: {
          enabled: false,
          multiple: false,
        }
    },
    legend: {
        position: 'inset'
    },
    grid: {
      y: {
        lines: [{value:0}] //Draws a horizontal line at y=0
      }
    },
    bar: {
        width: {
            ratio: 0.95 // this makes bar width 95% of length between ticks
        }
    },
    zoom: {
        enabled: true,
    },
    subchart: {
        show: false  //Shows a subchart that shows the region the primary chart is zoomed in to by overlay.
    }, 
    axis: {
        x: {
          type: 'category', //only for type 'category' the bars will be rescaled in width on zoom
          tick: {
            multiline: false,
            fit: false, //to make more x labels appear on zoom
            centered: true,
          },
        },
        y: {
          tick: {
            format: d3.format("s"),
          },
          //for some reason, there is an offset for our linegraph. This is prevented by the following lines
          //min: 0,
          //padding: { top: 0, bottom: 0 }
        }
    },
    //Style data points in linegraph
    point: {
        r: 0.2,
        focus: 
        {
          expand: {
            r: 4
          }
        }
    },
    tooltip: {
      format: {
        title: function (d) {
          var endValue = (parseFloat(greyvalue[d]) + binSize);
          endValue = endValue.toFixed(3);
          return 'Greyvalue: ' + greyvalue[d] + '...' + endValue;
        },
      }
    }
  });
}