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
