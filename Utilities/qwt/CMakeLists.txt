
IF(MITK_USE_QT) # only if MITK is built with Qt

PROJECT(QWT)

SET(QWT_MOC_HEADERS
  include/qwt_abstract_slider.h
  include/qwt_analog_clock.h
  include/qwt_compass.h
  include/qwt_counter.h
  include/qwt_dial.h
  include/qwt_dyngrid_layout.h
  include/qwt_knob.h
  include/qwt_legend.h
  include/qwt_legend_item.h
  include/qwt_magnifier.h
  include/qwt_panner.h
  include/qwt_picker.h
  include/qwt_plot_canvas.h
  include/qwt_plot.h
  include/qwt_plot_magnifier.h
  include/qwt_plot_panner.h
  include/qwt_plot_picker.h
  include/qwt_plot_rescaler.h
  include/qwt_plot_zoomer.h
  include/qwt_scale_widget.h
  include/qwt_slider.h
  include/qwt_text_label.h
  include/qwt_thermo.h
  include/qwt_wheel.h
)

SET(QWT_HEADERS 
  include/qwt_abstract_scale_draw.h  
  include/qwt_data.h  
  include/qwt_layout_metrics.h    
  include/qwt_plot_curve.h    
  include/qwt_plot_scaleitem.h    
  include/qwt_scale_widget.h
  include/qwt_abstract_scale.h   
  include/qwt_dial.h  
  include/qwt_legend.h    
  include/qwt_plot_dict.h    
  include/qwt_plot_spectrogram.h  
  include/qwt_slider.h
  include/qwt_abstract_slider.h   
  include/qwt_dial_needle.h  
  include/qwt_legend_item.h    
  include/qwt_plot_grid.h    
  include/qwt_plot_svgitem.h    
  include/qwt_spline.h
  include/qwt_analog_clock.h   
  include/qwt_double_interval.h  
  include/qwt_legend_itemmanager.h  
  include/qwt_plot.h    
  include/qwt_plot_zoomer.h
  include/qwt_plot_rescaler.h    
  include/qwt_symbol.h
  include/qwt_array.h   
  include/qwt_double_range.h  
  include/qwt_magnifier.h    
  include/qwt_plot_item.h    
  include/qwt_polygon.h    
  include/qwt_text_engine.h
  include/qwt_arrow_button.h   
  include/qwt_double_rect.h  
  include/qwt_math.h    
  include/qwt_plot_layout.h    
  include/qwt_raster_data.h    
  include/qwt_text.h
  include/qwt_clipper.h   
  include/qwt_dyngrid_layout.h   
  include/qwt_paint_buffer.h    
  include/qwt_plot_magnifier.h    
  include/qwt_rect.h    
  include/qwt_text_label.h
  include/qwt_color_map.h   
  include/qwt_event_pattern.h  
  include/qwt_painter.h    
  include/qwt_plot_marker.h    
  include/qwt_round_scale_draw.h  
  include/qwt_thermo.h
  include/qwt_compass.h   
  include/qwt_global.h  
  include/qwt_panner.h    
  include/qwt_plot_panner.h    
  include/qwt_scale_div.h    
  include/qwt_valuelist.h
  include/qwt_compass_rose.h   
  include/qwt.h  
  include/qwt_picker.h    
  include/qwt_plot_picker.h    
  include/qwt_scale_draw.h    
  include/qwt_wheel.h
  include/qwt_counter.h   
  include/qwt_interval_data.h  
  include/qwt_picker_machine.h    
  include/qwt_plot_printfilter.h  
  include/qwt_scale_engine.h
  include/qwt_curve_fitter.h   
  include/qwt_knob.h  
  include/qwt_plot_canvas.h    
  include/qwt_plot_rasteritem.h   
  include/qwt_scale_map.h

)
SET(QWT_SOURCES 
  src/qwt_abstract_scale.cpp
  src/qwt_double_interval.cpp
  src/qwt_panner.cpp
  src/qwt_plot_picker.cpp
  src/qwt_plot_rescaler.cpp
  src/qwt_scale_engine.cpp
  src/qwt_abstract_scale_draw.cpp
  src/qwt_double_range.cpp
  src/qwt_picker.cpp
  src/qwt_plot_print.cpp
  src/qwt_scale_map.cpp
  src/qwt_abstract_slider.cpp
  src/qwt_double_rect.cpp
  src/qwt_picker_machine.cpp
  src/qwt_plot_printfilter.cpp
  src/qwt_scale_widget.cpp
  src/qwt_analog_clock.cpp
  src/qwt_dyngrid_layout.cpp
  src/qwt_plot_axis.cpp
  src/qwt_plot_rasteritem.cpp
  src/qwt_slider.cpp
  src/qwt_arrow_button.cpp
  src/qwt_event_pattern.cpp
  src/qwt_plot_canvas.cpp
  src/qwt_plot_scaleitem.cpp
  src/qwt_spline.cpp
  src/qwt_clipper.cpp
  src/qwt_interval_data.cpp
  src/qwt_plot.cpp
  src/qwt_plot_spectrogram.cpp
  src/qwt_symbol.cpp
  src/qwt_color_map.cpp
  src/qwt_knob.cpp
  src/qwt_plot_curve.cpp
  src/qwt_plot_svgitem.cpp
  src/qwt_text.cpp
  src/qwt_compass.cpp
  src/qwt_layout_metrics.cpp
  src/qwt_plot_dict.cpp
  src/qwt_plot_xml.cpp
  src/qwt_text_engine.cpp
  src/qwt_compass_rose.cpp
  src/qwt_legend.cpp
  src/qwt_plot_grid.cpp
  src/qwt_plot_zoomer.cpp
  src/qwt_text_label.cpp
  src/qwt_counter.cpp
  src/qwt_legend_item.cpp
  src/qwt_plot_item.cpp
  src/qwt_raster_data.cpp
  src/qwt_thermo.cpp
  src/qwt_curve_fitter.cpp
  src/qwt_magnifier.cpp
  src/qwt_plot_layout.cpp
  src/qwt_rect.cpp
  src/qwt_wheel.cpp
  src/qwt_data.cpp
  src/qwt_math.cpp
  src/qwt_plot_magnifier.cpp
  src/qwt_round_scale_draw.cpp
  src/qwt_dial.cpp
  src/qwt_paint_buffer.cpp
  src/qwt_plot_marker.cpp
  src/qwt_scale_div.cpp
  src/qwt_dial_needle.cpp
  src/qwt_painter.cpp
  src/qwt_plot_panner.cpp
  src/qwt_scale_draw.cpp
)

SET(QT_MT_REQUIRED TRUE)
SET(QWT_INCLUDE_DIR ${QWT_SOURCE_DIR}/include CACHE PATH "MITK internal qwt include directory")
INCLUDE_DIRECTORIES(${QWT_INCLUDE_DIR})

FIND_PACKAGE(Qt4 REQUIRED)
SET(QT_USE_QTSVG 1)
INCLUDE(${QT_USE_FILE})
QT4_WRAP_CPP(QWT_SOURCES ${QWT_MOC_HEADERS})

ADD_DEFINITIONS(-DQWT_DLL)
ADD_DEFINITIONS(-DQWT_MAKEDLL)
ADD_LIBRARY(qwt ${QWT_SOURCES})
TARGET_LINK_LIBRARIES(qwt ${QT_LIBRARIES})

# install only if shared lib (for now)
INSTALL(TARGETS qwt${LIBPOSTFIX}
   RUNTIME DESTINATION bin   
)

ENDIF(MITK_USE_QT) # only if MITK is built with Qt

