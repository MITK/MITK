function showNextTip() {
  var tips = document.querySelectorAll('.tip');
  for (let i = 0; i < tips.length; ++i) {
    if (!tips[i].hasAttribute('hidden')) {
      tips[i].setAttribute('hidden', '');
      tips[(i + 1) % tips.length].removeAttribute('hidden');
      break;
    }
  }
}
