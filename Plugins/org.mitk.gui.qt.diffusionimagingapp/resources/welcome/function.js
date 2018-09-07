// If you want to create a new button you have to add some data (strings) to the following five arrays.
// Make sure that you add your data at the same position in each array.
// The buttons will be generated in order to the array's index. e.g. data at array's index '0' will generate the first button.

// enter the name of your module here
var moduleNames = new Array(
                            "Preprocessing",
                            "ODF Reconstruction/Signal Modelling",
                            "Diffusion Indices",
                            "Tractography",
                            "Fiber Processing",
                            "Connectomics",
                            "Fiberfox",
                            "IVIM & Kurtosis",
                            "Segmentation",
                            "MITK Downloads & News");

// add the MITK-link to your module
var moduleLinks = new Array(
                            "mitk://mitk.perspectives/org.mitk.perspectives.dwipreprocessing?clear=false",
                            "mitk://mitk.perspectives/org.mitk.perspectives.reconstruction1?clear=false",
                            "mitk://mitk.perspectives/org.mitk.perspectives.quantification?clear=false",
                            "mitk://mitk.perspectives/org.mitk.perspectives.tractography2?clear=false",
                            "mitk://mitk.perspectives/org.mitk.perspectives.fiberprocessings?clear=false",
                            "mitk://mitk.perspectives/org.mitk.perspectives.connectomics?clear=false",
                            "mitk://mitk.perspectives/org.mitk.perspectives.syntheticdata?clear=false",
                            "mitk://mitk.perspectives/org.mitk.perspectives.ivim?clear=false",
                            "mitk://mitk.perspectives/org.mitk.perspectives.segmentation?clear=false",
                            "http://www.mitk.org");

// add the filename of your icon for the module. Place the picture in subdirectory "pics".
// The picture's width should be 136 pixel; the height 123 pixel.
var picFilenames = new Array(
                             "12registration.png",
                             "02preprocessingreconstruction.png",
                             "03quantification.png",
                             "04tractography.png",
                             "05fiberprocessing.png",
                             "06connectomics.png",
                             "10softwarephantoms.png",
                             "07ivim.png",
                             "11segmentation.png",
                             "button_mitk.png");

// if you want to create an animated icon, add the name of your animated gif (placed in subdirectory "pics"). Otherwise enter an empty string "".
// The animation's width should be 136 pixel; the height 123 pixel.
var aniFilenames = new Array(
                             "12aregistration.png",
                             "02apreprocessingreconstruction.png",
                             "03aquantification.png",
                             "04atractography.png",
                             "05afiberprocessing.png",
                             "06aconnectomics.png",
                             "10asoftwarephantoms.png",
                             "07aivim.png",
                             "11asegmentation.png",
                             "button_mitka.png");

// if your module is not stable, you can mark it as experimental.
// just set true for experimental or false for stable.
var experimental = new Array(
                             false,
                             false,
                             false,
                             false,
                             false,
                             false,
                             false,
                             false,
                             false,
                             false);

//"MITK Diffusion allows partial volume analysis and the evaluation and exploration of Tract Based Spatial Statistic datasets (it does NOT reimplement the TBSS methods available in FSL). The Partial Volume Analysis view provides a very robust method for semi-automatic ROI analysis. It uses EM clustering to probabilistically segment fiber vs. non-fiber vs. partial volume.",

// add the description for your module. The description is displayed in a PopUp-window.
var moduleDescriptions = new Array(
            "You can use MITK Diffusion to preprocess diffusion-weighted images using image registration, head-motion correction, denoising and much more.",
            "MITK provides methods to estimate diffusion tensors or Q-balls using different reconstruction methods.",
            "MITK allows the calculation of different scalar measures for raw diffusion-weighted images (MD, ADC), tensors (Fractional Anisotropy, Relative Anisotropy, Axial Diffusivity, Radial Diffusivity) or ODFs (Generalized Fractional Anisotropy).",
            "The tractography components of MITK Diffusion implement several fiber tractography algorithms (global and streamline; tensor, dODF/fODF/FOD, peak based as well as machine learning based using the raw signal). The fiber bundle operations view enables fiber postprocessing in various ways. The fiber quantification view generates a variety of images from given fiber tracts, extracts the principal fiber directions and gives detailed information about the fiberbundle itself.",
            "The results obtained with any fiber tractography method can be postprocessed using tract dissection, filtering, clustering, coloring or SIFT2-like fitting methods.",
            "Connectomics aims at building graphs (nodes and edges) from a tractogram and applying graph theory for data analysis.",
            "The Fiberfox software phantom tool allows you to create synthetic diffusion-weighted MR images easily and with a wide variety of properties and options.",
            "You can use Intravoxel Incoherent Motion to estimate tissue perfusion on basis of diffusion measurements. Several models and interactive exploration of the data are supported.",
            "MITK Diffusion provides tools for image segmentation and diffusion unrelated image processing.",
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

//    // create text-link and add it to DIV-wrapper
//    bttnTxtLink = document.createElement("a");
////    bttnTxtLink.href = this.modLink;
//    bttnTxtLink.className = "txtLink";
//    bttnTxtLink.appendChild(document.createTextNode(this.modName));
//    bttnTxtLink.onclick = function(){showPopUpWindow();};
//    bttnTxtLink.id = "popup" + this.modName;
//    bttnWrapper.appendChild(bttnTxtLink);

    // create pop-up link for module description
    bttnPopUpLink = document.createElement("a");
    modName = this.modName;
    modDescr = this.modDescr;
    bttnPopUpLink.onclick = function(){showPopUpWindow();};
    bttnPopUpLink.className = "popUpLink";
    bttnPopUpLink.id = "popup" + this.modName;
    bttnPopUpLink.appendChild(document.createTextNode(this.modName));
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
//  contModLink.appendChild(moduleLink);
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

