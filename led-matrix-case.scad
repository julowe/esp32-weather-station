//Hub75 Matrix LED Case (with custom pcb on back)
//jkl 20210528

draftingFNs = 36;
renderFNs = 180;
$fn = draftingFNs;

caseFrontZpadding = 1;
screenWidth = 160+2-1.5; //too wide for calipers, assume double height, +2 for fit, -1.5 after full back test print
screenHeight = 80+1-0.5; //80.09 measured, 78.5 height at back of panel. 0.75mm diff each side, +1 for fit, -0.5 after full back test print
screenDepth = 15+caseFrontZpadding; //14.91 measured, +caseFrontZpadding for slight lip beyond leds

cornerScrewXinset = 17.25;//16.5 in from right side, assume same 0.75 slant, 17.25 in from right then
cornerScrewYinset = 8;//7.2mm from top, add 0.75, 8 down from top?
screwBuffer = 6; //extra material around screw hole in x & y directions

//using m3x16, fits t10 bit
screwShaftHoleDepth = 8; //expect some sagging of first few layers
screwShaftHoleRad = (3+1)/2; //m3 + some
screwHeadHoleDepth = 4;
screwHeadHoleRad = 5.6/2; //measured at 5.4 diam
caseExtraPaddingDepth = 6;

caseDepth = 40;//maybe 40 (bc of pcbs etc)
caseBackDepth = caseDepth - screenDepth; //caseExtraPaddingDepth + screwShaftHoleDepth;

caseWallThickness = 2;
caseWidth = screenWidth + caseWallThickness*2;
caseHeight = screenHeight + caseWallThickness*2;

//power plug center is 21mm from front with 15mm deep screen so 6
plugZoffset = caseDepth - screenDepth - 6; //this is to center of power plug
plugYoffset = 8;
plugZ = 12;
plugDiam = 7.25;
plugRad = plugDiam/2;
//plugX = 36; //this is without cables and wo silver barrel
plugX = 36-0.75; //this is without cables and wo silver barrel
plugWireZpadding = 5;
plugWireBackerX = 5;
//full width is 13-14
plugBackerY = 13;

//measured plug and ScreenZ = 26;

plugBackerZ = caseDepth - screenDepth - plugZ;
plugWireBackerZ = caseDepth - screenDepth - plugZ + plugWireZpadding;


middleBackerScrewXoffset = 92 + 2.15 - 1.5; //92 is to middle of screw hole, 2.15 mesaured adjustment factor after test print, 1.5 moved back after full back test print - NB offset from x=0
middleBackerX = 15;
middleBackerY = 12.25;
middleBackerXoffset = middleBackerScrewXoffset - middleBackerX/2;

keyholeScrewShaftDiam = 3.75;//measured at 3.45
keyholeScrewHeadDiam = 8;//measured at 6.75, but make large for future flexbility?
keyholeScrewHeadDepth = 5;//15.25 total length, 2.5 head depth, measured
keyholeBackDepth = 3; //amount between wall and screw head


/* TODO
x- fix screw hole offsets - measure x-gap btw case and matrix - gap is 1.5mm
- space on top for physical buttons
- keyhole hangers - make at same spot as top two corner screw holes
- back vents?
  - for vents, make screw hole/plug supports larger (by wallThickness) to go into case, this way they then won't be cut up by vent shapes that difference the main shell, but the vents can still perforate most of case wall edge
  - oooooh or, just make vents with slicer modifaction parts , honeycomb infill with no perimeters? octogram spiral looks good for x=constant walls, honeycomb for z=0? bottom y=0 type?
- hole in top for usb cord, and dust cover

** Maybe **

-make wall that holds power lug in, 2mm higher

*/


//outer wall
difference(){
    //outside
    cube([caseWidth,caseHeight,caseDepth]);
    
    //remove inside cube
    //TODO don't remove some of back wall and perforate with vents
    translate([caseWallThickness,caseWallThickness,0]){
        cube([screenWidth,screenHeight,caseDepth]);
    }
    
    //remove power plug cylinder
    translate([caseWidth,caseWallThickness+plugYoffset,plugZoffset]){
        rotate([0,-90,0]){
            cylinder(caseWallThickness,plugRad,plugRad);
        }
    }
}


//bottom right corner, power plug backer/holder
difference(){
    //main block that is behind wires
    translate([caseWidth-caseWallThickness-(plugX+plugWireBackerX),caseWallThickness,0]){
        cube([plugX+plugWireBackerX,plugBackerY,plugWireBackerZ]);
    }
    
    //remove block for main plug body
    translate([caseWidth-caseWallThickness-plugX,caseWallThickness,plugBackerZ]){
        cube([plugX,cornerScrewYinset+screwBuffer,plugZ]);
    }
}


//lower left corner
difference(){
    translate([caseWallThickness,caseWallThickness,0]){
        cube([cornerScrewXinset+screwBuffer,cornerScrewYinset+screwBuffer,caseBackDepth]);
    }
    //screwshaft
    translate([caseWallThickness+cornerScrewXinset,caseWallThickness+cornerScrewYinset,caseBackDepth-screwShaftHoleDepth]){
        cylinder(screwShaftHoleDepth, screwShaftHoleRad, screwShaftHoleRad);
    }
    //screwhead
    translate([caseWallThickness+cornerScrewXinset,caseWallThickness+cornerScrewYinset,caseBackDepth-screwShaftHoleDepth-screwHeadHoleDepth]){
        cylinder(screwHeadHoleDepth, screwHeadHoleRad, screwHeadHoleRad);
    }
    //screwshaft
    translate([caseWallThickness+cornerScrewXinset,caseWallThickness+cornerScrewYinset,0]){
        cylinder(caseBackDepth-screwHeadHoleDepth-screwShaftHoleDepth, screwHeadHoleRad+1, screwHeadHoleRad);
    }
}

//lower middle support
difference(){
    translate([caseWallThickness+middleBackerXoffset,caseWallThickness,0]){
        cube([middleBackerX,middleBackerY,caseBackDepth]);
    }
    //screwshaft
    translate([caseWallThickness+middleBackerScrewXoffset,caseWallThickness+cornerScrewYinset,caseBackDepth-screwShaftHoleDepth]){
        cylinder(screwShaftHoleDepth, screwShaftHoleRad, screwShaftHoleRad);
    }
    //screwhead
    translate([caseWallThickness+middleBackerScrewXoffset,caseWallThickness+cornerScrewYinset,caseBackDepth-screwShaftHoleDepth-screwHeadHoleDepth]){
        cylinder(screwHeadHoleDepth, screwHeadHoleRad, screwHeadHoleRad);
    }
    //screwshaft
    translate([caseWallThickness+middleBackerScrewXoffset,caseWallThickness+cornerScrewYinset,0]){
        cylinder(caseBackDepth-screwHeadHoleDepth-screwShaftHoleDepth, screwHeadHoleRad+1, screwHeadHoleRad);
    }
}

//top right corner
difference(){
    translate([caseWidth-caseWallThickness-(cornerScrewXinset+screwBuffer),caseHeight-caseWallThickness-(cornerScrewYinset+screwBuffer),0]){
        cube([cornerScrewXinset+screwBuffer,cornerScrewYinset+screwBuffer,caseBackDepth]);
    }
    //screwshaft
    translate([caseWidth-caseWallThickness-cornerScrewXinset,caseHeight-caseWallThickness-cornerScrewYinset,caseBackDepth-screwShaftHoleDepth]){
        cylinder(screwShaftHoleDepth, screwShaftHoleRad, screwShaftHoleRad);
    }
    //screwhead
    translate([caseWidth-caseWallThickness-cornerScrewXinset,caseHeight-caseWallThickness-cornerScrewYinset,caseBackDepth-screwShaftHoleDepth-screwHeadHoleDepth]){
        cylinder(screwHeadHoleDepth, screwHeadHoleRad, screwHeadHoleRad);
    }
    //screwshaft
    translate([caseWidth-caseWallThickness-cornerScrewXinset,caseHeight-caseWallThickness-cornerScrewYinset,0]){
        cylinder(caseBackDepth-screwHeadHoleDepth-screwShaftHoleDepth, screwHeadHoleRad+1, screwHeadHoleRad);
    }
}

//top left corner
difference(){
    translate([caseWallThickness,caseHeight-caseWallThickness-(cornerScrewYinset+screwBuffer),0]){
        cube([cornerScrewXinset+screwBuffer,cornerScrewYinset+screwBuffer,caseBackDepth]);
    }
    //screwshaft
    translate([caseWallThickness+cornerScrewXinset,caseHeight-caseWallThickness-cornerScrewYinset,caseBackDepth-screwShaftHoleDepth]){
        cylinder(screwShaftHoleDepth, screwShaftHoleRad, screwShaftHoleRad);
    }
    //screwhead
    translate([caseWallThickness+cornerScrewXinset,caseHeight-caseWallThickness-cornerScrewYinset,caseBackDepth-screwShaftHoleDepth-screwHeadHoleDepth]){
        cylinder(screwHeadHoleDepth, screwHeadHoleRad, screwHeadHoleRad);
    }
    //screwshaft
    translate([caseWallThickness+cornerScrewXinset,caseHeight-caseWallThickness-cornerScrewYinset,0]){
        cylinder(caseBackDepth-screwHeadHoleDepth-screwShaftHoleDepth, screwHeadHoleRad+1, screwHeadHoleRad);
    }
}

