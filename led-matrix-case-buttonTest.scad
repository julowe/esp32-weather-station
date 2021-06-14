//test piece for button slide ins

buttonDepthPadding = 5; //allowance to make 'slide in' work
buttonHeight = 3; //includes small black circle son top of button - at 4 corners
buttonWidth = 11.5;
buttonDepth = buttonWidth + 10; //extra dimension for cutting through back wall - to allow 'slide in'

buttonPegWidth = 1;
buttonPegHeight = 2;

buttonLegWidth = 1.5;
buttonLegHeight = 4 + buttonDepthPadding; //mesaured at 4, +x for extra working room
buttonLegOverlap = 1; //amount of metal in button body in Z direction

buttonTopEdgeWidth = 2; //space between edge and bewginning of button mechanism that is depressed
buttonTopMechanismRidgeWidth = 1; //distance between outer diameter of button depression mechaism and stem
buttonTopMechanismWidth = buttonWidth - buttonTopEdgeWidth*2;
buttonTopMechanismDepth = buttonDepth - buttonTopEdgeWidth; //buttonTopMechanismWidth + buttonDepthPadding;
buttonTopMechanismHeight = 10;
//button stem then has button clipped on, making it full width of base.

//buttonStemWidth = 4.5;
//buttonStemOffset = 3.5;
//buttonStemHeight = 10;


/* TODO

- decide where z=0 is

*/

//main button body
cube([buttonWidth,buttonDepth,buttonHeight]);

//button circle base
translate([buttonTopEdgeWidth,buttonTopEdgeWidth,buttonHeight]){
    cube([buttonTopMechanismWidth,buttonTopMechanismDepth,buttonTopMechanismHeight]);
}

////button stem
//translate([buttonStemOffset,buttonStemOffset,buttonHeight+buttonTopMechanismHeight]){
//    cube([buttonStemWidth,buttonDepth,buttonStemHeight]);
//}

//buttonPegWidth = 1;
//buttonPegHeight = 2;

//space for lttle pegs on bottom of button
translate([buttonWidth/2 - buttonPegWidth/2,0,-buttonPegHeight]){
    cube([buttonPegWidth,buttonDepth,buttonPegHeight]);
}


//left button legs
translate([-buttonLegWidth,0,-buttonLegHeight+buttonLegOverlap]){
    cube([buttonLegWidth,buttonDepth,buttonLegHeight]);
}
//right button legs
translate([buttonWidth,0,-buttonLegHeight+buttonLegOverlap]){
    cube([buttonLegWidth,buttonDepth,buttonLegHeight]);
}


translate([0,0,0]){
    cube([0,0,0]);
}