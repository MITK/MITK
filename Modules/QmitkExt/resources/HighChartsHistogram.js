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

var connected = false;
var chart;
var histData;

if (!connected)
{
  connected = true;
  histogramData.DataChanged.connect(updateHistogram);
  histogramData.GraphChanged.connect(toggleGraph);
}

$(function () {
  chart = new Highcharts.Chart({
    chart: {
      renderTo: 'container',
      resetZoomButton: {
        position: {
          x:-10,
          y:10
        },
        relativeTo: 'chart'
      },
      animation: {
        duration: 1500,
      },
      zoomType: 'x'
    },
    title: {
      text: null
    },
    plotOptions: {
      column: {
        borderWidth: 0.0,
        shadow: false,
        turboThreshold: 500.0,
        enableMouseTracking: false
      },
      line: {
        shadow: false,
        lineWidth: 1.0,
        marker: {
          enabled: false
        },
        stickyTracking: false,
        visible: false
      }
    },
    xAxis: {
      title: {
        text: null
      },
      allowDecimals: false,
      endOnTick: true
    },
    yAxis: {
      title: {
        text: null
      },
      allowDecimals: false,
      endOnTick: true
    },
    tooltip: {
      shadow: false,
      formatter: function() {
        if (!histogramData.intensityProfile)
        {
          return 'Greyvalue: '+this.x+'<br/>'+'Frequency: '+this.y;
        }
        else
        {
          return 'Distance: '+Math.round(this.x*100)/100+' mm<br/>'+'Intensity: '+this.y;
        }
      }
    },
    series: [{
      type: 'column',
      data: histData,
      name: 'Barchart'
    }, {
      type: 'line',
      data: histData,
      name: 'Lineplot'
    }],
    credits: {
      enabled: false
    },
    navigation: {
      buttonOptions: {
        enabled: false
      }
    },
    legend: {
      enabled: false
    }
  });
});

updateHistogram();

function updateHistogram() {
  if (histogramData.useLineGraph)
  {
    chart.series[0].hide();
    chart.series[1].show();
  }
  else
  {
    chart.series[0].show();
    chart.series[1].hide();
  }
  histData = [];
  for (var i = 0; i < histogramData.frequency.length; i++)
  {
    var tempArray = [histogramData.measurement[i], histogramData.frequency[i]]
    histData[i] = tempArray;
  }
  chart.series[0].setData(histData);
  chart.series[1].setData(histData);
  chart.redraw();
}

function toggleGraph()
{
  if (histogramData.useLineGraph)
  {
    chart.series[0].hide();
    chart.series[1].show();
  }
  else
  {
    chart.series[0].show();
    chart.series[1].hide();
  }
}
