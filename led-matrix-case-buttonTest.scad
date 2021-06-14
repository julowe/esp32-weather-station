//test piece for button slide ins


buttonVoidTest();


module buttonShape(){
    
    buttonDepthPadding = 5; //allowance to make 'slide in' work
    buttonHeight = 4.25; //includes small black circle son top of button - at 4 corners
    buttonWidth = 12; //at 12 the y-axis fit is a bit tight, but holds firm. add 0.5 for looser fit
    buttonDepth = buttonWidth + 10; //extra dimension for cutting through back wall - to allow 'slide in'

    buttonPegWidth = 2;
    buttonPegHeight = 2;

    buttonLegWidth = 1.5;
    buttonLegHeight = 4 + buttonDepthPadding; //mesaured at 4, +x for extra working room
    buttonLegOverlap = 1.5; //amount of metal in button body in Z direction

    buttonTopEdgeWidth = 2; //space between edge and bewginning of button mechanism that is depressed
    buttonTopMechanismRidgeWidth = 1; //distance between outer diameter of button depression mechaism and stem
    buttonTopMechanismWidth = buttonWidth - buttonTopEdgeWidth*2;
    buttonTopMechanismDepth = buttonDepth - buttonTopEdgeWidth; //buttonTopMechanismWidth + buttonDepthPadding;
    buttonTopMechanismHeight = 10;
    
    //button stem then has button clipped on, making it full width of base.
    buttomStemHeight = 1.5; //z space betwen button mechanism base and bottom of button top (what is pressed)
    buttonTopWidth = 10.5; //meaured at 10
    buttonTopHeight = 3.5; //measured at 3.17 - this hsoudl really be open to air anyway in the z direction
    buttonTopYsetback = 0.5;
    
    //main button body
    cube([buttonWidth,buttonDepth,buttonHeight]);

    //button circle base
    translate([buttonTopEdgeWidth,buttonTopEdgeWidth,buttonHeight]){
        cube([buttonTopMechanismWidth,buttonTopMechanismDepth,buttonTopMechanismHeight]);
    }

    //space for little pegs on bottom of button
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
    
    //button top
    translate([buttonWidth/2-buttonTopWidth/2,buttonTopYsetback,buttonHeight+buttomStemHeight]){
        cube([buttonTopWidth,buttonTopMechanismDepth,buttonTopMechanismHeight]);
    }
}

module buttonVoidTest(){
    difference(){
        translate([-3.5,-2,-11]){
            cube([12 + 3 + 2*2,22,9+4.25+4+2*2]);
        }
        //translate([-(buttonLegWidth + 2),-2,-(buttonLegHeight+2)]){
        //    cube([buttonWidth + buttonLegWidth*2 + 2*2,buttonDepth,buttonLegHeight+buttonHeight+buttonTopMechanismHeight+2*2]);
        //}

        buttonShape();
    }
}