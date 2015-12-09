// If you want to create a new button you have to add some data (strings) to the following five arrays.
// Make sure that you add your data at the same position in each array.
// The buttons will be generated in order to the array's index. e.g. data at array's index '0' will generate the first button.

// enter the name of your module here
var moduleNames = new Array("MITK Website",
                            "Tutorials Website");

// add the MITK-link to your module
var moduleLinks = new Array("http://www.mitk.org/",
                            "http://mitk.org/wiki/Tutorials");

// add the filename of your icon for the module. Place the picture in subdirectory "pics".
// The picture's width should be 136 pixel; the height 123 pixel.
var picFilenames = new Array("button_mitk.png",
                             "button_mitk.png");

// if your module is not stable, you can mark it as experimental.
// just set true for experimental or false for stable.
var experimental = new Array(false,
                             false);

// add the description for your module. The description is displayed in a PopUp-window.
var moduleDescriptions = new Array("Open the MITK website in an external browser.",
                                   "Open the MITK tutorials overview in an external browser.");

var bttns = new Array();

var d = document, da = d.all;

// holds id of current mouseover-HTML-element
var currentTarget;

// get the id of current mouseover-HTML-element
d.onmouseover = function(o){
  var e = da ? event.srcElement : o.target;
  currentTarget = e.id;
}


// build-function called by onload-event in HTML-document
function createButtons(){

  for (i=0; i < moduleNames.length; i++){
    bttns[i] = new Button(moduleNames[i],moduleLinks[i], picFilenames[i], moduleDescriptions[i]);
    bttns[i].createButton();
  }

  for (i=0; i < moduleNames.length; i++){

    if(experimental[i]){
      setExperimental(i);
    }
  }

  createClearFloat();
}

// Class Button
function Button(moduleName, moduleLink, picFilename, moduleDescr){

  // Properties
  this.bttnID = "bttn" + moduleName.replace(/\s/g,'');
  this.modName = moduleName;
  this.modLink = moduleLink;
  this.picPath = "qrc:/org.mitk.gui.qt.welcomescreen/pics/" + picFilename;
  this.modDescr = moduleDescr;

  // Methods
  this.createButton = function(){

    // get DIV-wrapper for Button and append it to HTML-document
    bttnWrapper = this.createWrapper();
    document.getElementById("generatedButtonField").appendChild(bttnWrapper);

    // Create a button and append it to document
    this.createCSSClass();
    bttnPicLink = this.createPicLink();
    bttnPicLink.href = this.modLink;
    bttnWrapper.appendChild(bttnPicLink);

    // create text-link and add it to DIV-wrapper
    bttnTxtLink = document.createElement("a");
    bttnTxtLink.onclick = function(){openPage();};
    bttnTxtLink.href = this.modLink;
    bttnTxtLink.className = "textLink";
    bttnTxtLink.appendChild(document.createTextNode(this.modName));
    bttnWrapper.appendChild(bttnTxtLink);

    // create pop-up link for module description
    if (this.modDescr.length != 0) {
      bttnPopUpLink = document.createElement("a");
      modName = this.modName;
      modDescr = this.modDescr;
      bttnPopUpLink.onclick = function(){showPopUpWindow();};
      bttnPopUpLink.className = "popUpLink";
      bttnPopUpLink.id = "popup" + this.modName;
      bttnPopUpLink.appendChild(document.createTextNode("more info >>"));
      bttnWrapper.appendChild(document.createElement("br"));
      bttnWrapper.appendChild(bttnPopUpLink);
    }

    return bttnPicLink;
  }

  this.createWrapper = function(){
    bttnWrapper = document.createElement("div");
    bttnWrapper.id = "wrapper" + this.modName;
    bttnWrapper.className = "bttnWrap";

    return bttnWrapper;
  }

  this.createPicLink = function(){
    picLink = document.createElement("a");
    picLink.id = "link" + this.modName;
    picLink.href = this.modLink;
    picLink.className = "genericButton " + this.bttnID;

    return picLink;
  }

  this.createCSSClass = function(){
    var style = document.createElement('style');
    style.type = 'text/css';
    style.innerHTML = '.' + this.bttnID +' { background-image: url(' + this.picPath +'); }';
    document.getElementsByTagName('head')[0].appendChild(style);
  }

}


function showPopUpWindow(){

  // modules position in array?
  modulePos = getPos(currentTarget,"popup");

  // get reference to anchor-element in HTML-document
  popUpAnchor = document.getElementById("popupAnchor");

  // check if a popUp is open
  if(popUpAnchor.hasChildNodes()){
    // if a popUp is open, remove it!
    popUpAnchor.removeChild(document.getElementById("popup"));
  }

  // create new container for popUp
  container = document.createElement("div");
  container.id = "popup";
  container.align = "right";

  // append popUp-container to HTML-document
  popUpAnchor.appendChild(container);

  // create close-button and append it to popUp-container
  bttnClose = document.createElement("img");
  bttnClose.src = "pics/popup_bttn_close.png";
  bttnClose.id = "bttnClose";
  bttnClose.onclick = function(){closeInfoWindow();};
  container.appendChild(bttnClose);

  // create container for content-elements
  contHeadline = document.createElement("div");
  contHeadline.id = "contHeadline";
  contDescription = document.createElement("div");
  contDescription.id = "contDescription";
  contModLink = document.createElement("div");
  contModLink.id = "contModLink";

  // append content-container to popUp-container
  container.appendChild(contHeadline);
  container.appendChild(contDescription);
  container.appendChild(contModLink);

  // create text-elements with content
  headline = document.createTextNode(moduleNames[modulePos] + " ");
  description = document.createTextNode(moduleDescriptions[modulePos]);
  moduleLink = document.createElement("a");
  moduleLink.href = moduleLinks[modulePos] ;
  moduleLink.className = 'moduleLink';
  moduleLinkTxt = document.createTextNode("Click here to open '" + moduleNames[modulePos].toLowerCase() + "'");
  moduleLink.appendChild(moduleLinkTxt);

  // append text-elements to their container
  contHeadline.appendChild(headline);
  contDescription.appendChild(description);
  contModLink.appendChild(moduleLink);
}

function getPos(id,prefix){

  if(prefix == "popup"){
    targetID = id.slice(5);
  }else{
    if(prefix == "bttn"){
      targetID = id.slice(4);
    }
  }

  for(i=0; i < moduleNames.length; i++ ){
    if(moduleNames[i] == targetID){
      return i;
    }
  }
}

function setExperimental(modPos){
  linkID = "link" + moduleNames[modPos];

  expPic = document.createElement("img");
  expPic.src = "pics/experimental.png";
  expPic.className = "expPic";

  document.getElementById(linkID).appendChild(expPic);
}


function openPage(){
  window.open("http://www.mitk.org","_blank");
}

// function to close PopUp-window
function closeInfoWindow(){
  popUpAnchor = document.getElementById("popupAnchor");
  popUpAnchor.removeChild(document.getElementById("popup"));
}

function createClearFloat(){
  cf = document.createElement("div");
  cf.className = "clearfloat";
  document.getElementById("generatedButtonField").appendChild(cf);
}
