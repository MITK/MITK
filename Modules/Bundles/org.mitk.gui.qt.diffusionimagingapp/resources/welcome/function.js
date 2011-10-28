// If you want to create a new button you have to add some data (strings) to the following five arrays.
// Make sure that you add your data at the same position in each array.
// The buttons will be generated in order to the array's index. e.g. data at array's index '0' will generate the first button.

// enter the name of your module here
var moduleNames = new Array("Dicom Import",
                            "Preprocessing & Reconstruction",
                            "Quantification",
                            "Tractography",
                            "TBSS",
                            "Connectomics",
                            "IVIM",
                            "Volume Visualization",
                            "Screenshots & Movies",
                            "MITK Downloads & News");

// add the MITK-link to your module
var moduleLinks = new Array("mitk://mitk.perspectives/org.mitk.diffusionimagingapp.perspectives.dicomimport?clear=false",
                            "mitk://mitk.perspectives/org.mitk.diffusionimagingapp.perspectives.preprocessingreconstruction?clear=false",
                            "mitk://mitk.perspectives/org.mitk.diffusionimagingapp.perspectives.quantification?clear=false",
                            "mitk://mitk.perspectives/org.mitk.diffusionimagingapp.perspectives.tractography?clear=false",
                            "mitk://mitk.perspectives/org.mitk.diffusionimagingapp.perspectives.tbss?clear=false",
                            "mitk://mitk.perspectives/org.mitk.diffusionimagingapp.perspectives.connectomics?clear=false",
                            "mitk://mitk.perspectives/org.mitk.diffusionimagingapp.perspectives.ivim?clear=false",
                            "mitk://mitk.perspectives/org.mitk.diffusionimagingapp.perspectives.volumevisualization?clear=false",
                            "mitk://mitk.perspectives/org.mitk.diffusionimagingapp.perspectives.screenshotsmovies?clear=false",
                            "http://www.mitk.org");

// add the filename of your icon for the module. Place the picture in subdirectory "pics".
// The picture's width should be 136 pixel; the height 123 pixel.
var picFilenames = new Array("01dicomimport.png",
                             "02preprocessingreconstruction.png",
                             "03quantification.png",
                             "04tractography.png",
                             "05tbss.png",
                             "06connectomics.png",
                             "07ivim.png",
                             "08volumevisualization.png",
                             "09screenshotsmovies.png",
                             "button_mitk.png");

// if you want to create an animated icon, add the name of your animated gif (placed in subdirectory "pics"). Otherwise enter an empty string "".
// The animation's width should be 136 pixel; the height 123 pixel.
var aniFilenames = new Array("01adicomimport.png",
                             "02apreprocessingreconstruction.png",
                             "03aquantification.png",
                             "04atractography.png",
                             "05atbss.png",
                             "06aconnectomics.png",
                             "07aivim.png",
                             "08avolumevisualization.png",
                             "09ascreenshotsmovies.png",
                             "button_mitka.png");

// if your module is not stable, you can mark it as experimental.
// just set true for experimental or false for stable.
var experimental = new Array(false,
                             false,
                             false,
                             false,
                             false,
                             false,
                             false,
                             false,
                             false,
                             false);

// add the description for your module. The description is displayed in a PopUp-window.
var moduleDescriptions = new Array("'Dicom Import' supports direct import of Siemens diffusion weighted DICOM files.",
                            "'Preprocessing & Reconstruction' lets you estimate diffusion tensors or q-balls using different reconstruction methods. It also includes an ODF-details view and preprocessing steps like gradient avaraging, baseline image extraction, and binary mask estimation.",
                            "'Quantification' calculates scalar indices (FA, GFA, ...) and allows ROI-analysis in 2D or 3D. The Partial Volume Analysis view provides a very robust method for semi-automatic ROI analysis. It uses EM clustering to probabilistically segment fiber vs. non-fiber vs. partial volume.",
                            "'Tractography' implements a global yet computationally efficient tracking algorithm (gibbs-tracking). In addition it provides a stochastic tractography method. The fiber bundle operations view allows for extraction, join, and substraction of bundles. It also generates a variety of images from given fiber tracts.",
                            "'TBSS' does *not* reimplement the original Tract Based Spatial Statistics method available in FSL. Aim of this module is to read TBSS output and to provide means to interactively explore and evaluate the datasets.",
                            "'Connectomics' aims at building graphs (nodes and edges) from a global tractogram and applying graph theory for data analysis. fMRI-data might be incorporated in future.",
                            "'IVIM' stands for Intravoxel Incoherent Motion. The technique allows for the estimation of tissue perfusion on basis of diffusion measurements. Several models are supported by this module. Interactive exploration of the data is supported.",
                            "'Volume Visualization' provides easy to use and fast volume rendering of images. Transfer-functions can be loaded, saved, and adapted interactively.",
                            "'Screenshots & Movies' provides means to capture high resolution screenshots as well as movies of the 3D sceen.",
                            "Open the MITK website in an external browser.");

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
    bttns[i] = new Button(moduleNames[i],moduleLinks[i], picFilenames[i], aniFilenames[i],moduleDescriptions[i]);
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
function Button(moduleName, moduleLink, picFilename, aniFilename, moduleDescr){

  // Properties
  this.bttnID = "bttn" + moduleName;
  this.modName = moduleName;
  this.modLink = moduleLink;
  this.picPath = "pics/" + picFilename;
  this.aniPath = "pics/" + aniFilename;
  this.modDescr = moduleDescr;

  // Methods
  this.createButton = function(){

    // get DIV-wrapper for Button and append it to HTML-document
    bttnWrapper = this.createWrapper();
    document.getElementById("bttnField").appendChild(bttnWrapper);

    // get link-element for picture and append it to DIV-wrapper
    bttnPicLink = this.createPicLink();
    bttnWrapper.appendChild(bttnPicLink);

    // set HTML attributes for button-element
    bttn = document.createElement("img");
    bttn.src = this.picPath;
    bttn.id = this.bttnID;
    bttn.className = "modBttn";
    bttn.height = 123;
    bttn.width = 136;
    bttn.onmouseover = function(){startAni(this.id);};
    bttn.onmouseout = function(){stopAni(this.id);};

    // append button to link-element
    bttnPicLink.appendChild(bttn);

    // create text-link and add it to DIV-wrapper
    bttnTxtLink = document.createElement("a");
    bttnTxtLink.href = this.modLink;
    bttnTxtLink.className = "txtLink";
    bttnTxtLink.appendChild(document.createTextNode(this.modName));
    bttnWrapper.appendChild(bttnTxtLink);

    // create pop-up link for module description
    bttnPopUpLink = document.createElement("a");
    modName = this.modName;
    modDescr = this.modDescr;
    bttnPopUpLink.onclick = function(){showPopUpWindow();};
    bttnPopUpLink.className = "popUpLink";
    bttnPopUpLink.id = "popup" + this.modName;
    bttnPopUpLink.appendChild(document.createTextNode("more info >>"));
    bttnWrapper.appendChild(document.createElement("br"));
    bttnWrapper.appendChild(bttnPopUpLink);

    return bttn;
  }

  this.createWrapper = function(){
    bttnWrapper = document.createElement("div");
    bttnWrapper.id = "wrapper" + this.modName;
    bttnWrapper.className = "bttnWrap";

    return bttnWrapper;
  }

  this.createPicLink = function(){
    picLink = document.createElement("a");
    picLink.href = this.modLink;
    picLink.id = "link" + this.modName;

    return picLink;
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
  //alert(bttns[modPos].bttnID);
  expPic.onmouseover = function(){startAni(bttns[modPos].bttnID);changeToHover(bttns[modPos].bttnID);};
  expPic.onmouseout = function(){stopAni(bttns[modPos].bttnID);changeToNormal(bttns[modPos].bttnID);};

  document.getElementById(linkID).appendChild(expPic);
}

function changeToHover(targetId){
  bttn = document.getElementById(targetId);
  bttn.className = "modBttnHover";
}

function changeToNormal(targetId){
  bttn = document.getElementById(targetId);
  bttn.className = "modBttn";
}

// function to close PopUp-window
function closeInfoWindow(){
  popUpAnchor = document.getElementById("popupAnchor");
  popUpAnchor.removeChild(document.getElementById("popup"));
}

function createClearFloat(){
  cf = document.createElement("div");
  cf.className = "clearfloat";
  document.getElementById("bttnField").appendChild(cf);
}

startAni = function(targetId){
  modulePos = getPos(targetId,"bttn");

  if(aniFilenames[modulePos] != ''){
    bttn = document.getElementById(targetId);
    bttn.src = "pics/" + aniFilenames[modulePos];
  }
}

stopAni = function(targetId){
  modulePos = getPos(targetId,"bttn");

  bttn = document.getElementById(targetId);
  bttn.src = "pics/" + picFilenames[modulePos];
}

