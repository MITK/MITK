function changeTheme(color) {
  if (color == 'dark') {
    link = document.getElementsByTagName("link")[0];
    link.href = "Chart_dark.css";
  }
  else
  {
    link = document.getElementsByTagName("link")[0];
    link.href = "Chart.css";
  }
};
