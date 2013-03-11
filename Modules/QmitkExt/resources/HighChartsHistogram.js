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
      text: 'Histogram'
    },
    plotOptions: {
      column: {
        borderWidth: 0.0,
        shadow: false,
        turboThreshold: 500.0,
        enableMouseTracking: false
      },
      spline: {
        shadow: false,
        lineWidth: 1.0,
        marker: {
          enabled: false
        },
        visible: false
      }
    },
    xAxis: {
      title: {
        text: 'Greyvalue'
      },
      allowDecimals: false,
      endOnTick: true
    },
    yAxis: {
      title: {
        text: 'Frequency (px)'
      },
      allowDecimals: false,
      endOnTick: true
    },
    tooltip: {
      shadow: false,
      formatter: function() {
        return 'Greyvalue: '+this.x+'<br/>'+'Frequency: '+this.y;
      }
    },
    series: [{
      type: 'column',
      data: histData,
      name: 'Barchart'
    }, {
      type: 'spline',
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
      enabled: true
    }
  });
});

updateHistogram();

function updateHistogram() {
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
