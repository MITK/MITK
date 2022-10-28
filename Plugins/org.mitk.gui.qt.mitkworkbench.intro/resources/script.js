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

function showRandomTip() {
  var tips = document.querySelectorAll('.tip');
  var show = Math.floor(Math.random() * tips.length);
  for (let i = 0; i < tips.length; ++i) {
    if (i != show) {
      tips[i].setAttribute('hidden', '');
    } else {
      tips[i].removeAttribute('hidden');
    }
  }
}

function showTips() {
  var tips = document.querySelector('.tips');
  tips.removeAttribute('hidden');
}

function hideTips() {
  var tips = document.querySelector('.tips');
  tips.setAttribute('hidden', '');
}
