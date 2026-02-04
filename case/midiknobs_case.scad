brd_xsz = 100.5;
brd_ysz = 120.5;
brd_overhead_low = 9;
brd_overhead_tall = 9;
brd_thick = 1.5;
brd_h = 7;
pot_d = 7.5;
rot_d = 10;
bn_d = 13;
low_part = 64;
thick = 1.6;
ispconn_ysz = 10;
ispconn_xsz = 17;
led_d = 5;

dip_xsz = 13;
dip_ysz = 10;

pot_yoffs = 16;
pot1_xoffs = 17;
pot2_xoffs = 50.5;
rot_xoffs = 83;
bn_yoffs = 45;
led_xoffs = 78;
led_yoffs = 43;
led2_xoffs = 36;
led2_yoffs = 84;

isp_xoffs = 77.5;
isp_yoffs = 53;
dip_xoffs = isp_xoffs;
dip_yoffs = 67;

din_hole_d = 19;
din_offs = 25;
din_d = 21;

pwrconn_offs = 35;
pwrconn_ysz = 12;
pwrconn_h = 7.5;

bolt_yoffs = 5.5;
bolt_yoffs2 = brd_ysz - 7;
bolt_xoffs = 7;
bolt_d = 2.6;
bolt_head_d = 4.5;

bevel = 1.5;

box_xsz = brd_xsz + thick * 2;
box_ysz = brd_ysz + thick * 2;
box_h1 = brd_overhead_low + brd_h + thick * 2;
box_h2 = brd_overhead_tall + brd_h + thick * 2;


module din_conn()
{
	in_h = 8;
	out_h = 5.5;
	total_h = in_h + out_h;

	translate([din_offs, -brd_ysz+0.01, -brd_h+din_d/2]) {
		translate([0, in_h, 0])
		rotate([90, 0, 0])
		cylinder(h=total_h, d=din_hole_d, $fn=100);
		
		translate([0, 2.01, 0])
		rotate([90, 0, 0])
		cylinder(h=2, d=din_d, $fn=100);
		
		translate([0, -1.4/2, -din_hole_d/2])
		cube([3, 1.4, 1.5], center=true);
	}
}


module top_holes()
{
	$fn = 60;
	
	translate([pot1_xoffs, -pot_yoffs, 0])
	cylinder(h=box_h1, d=pot_d);
	
	translate([pot2_xoffs, -pot_yoffs, 0])
	cylinder(h=box_h1, d=pot_d);
	
	translate([rot_xoffs, -pot_yoffs, 0])
	cylinder(h=box_h1, d=rot_d);
	
	translate([pot1_xoffs, -bn_yoffs, 0])
	cylinder(h=box_h1, d=bn_d);
	
	translate([pot2_xoffs, -bn_yoffs, 0])
	cylinder(h=box_h1, d=bn_d);
	
	translate([led_xoffs, -led_yoffs, brd_overhead_low - 0.1])
	cylinder(h=thick+0.2, d1=led_d * 1.5, d2=led_d);
	
	translate([led2_xoffs, -led2_yoffs, brd_overhead_tall - 0.1])
	cylinder(h=thick+0.2, d1=led_d * 1.5, d2=led_d);
	
	translate([isp_xoffs, -isp_yoffs, 0])
	mirror([0, 1, 0])
	cube([ispconn_xsz, ispconn_ysz, box_h2]);
	
	translate([dip_xoffs, -dip_yoffs, 0])
	mirror([0, 1, 0])
	cube([dip_xsz, dip_ysz, box_h2]);
	
	translate([brd_xsz - 1, -pwrconn_offs, -0.01])
	mirror([0, 1, 0])
	cube([thick + 2, pwrconn_ysz, pwrconn_h]);
}

module bolt()
{
	cylinder(box_h2 + 2, d=bolt_d, $fn=40);
	
	cylinder(2, d=4.8, $fn=6);
	
	head_h = 1;
	translate([0, 0, box_h1-head_h + 0.01])
	cylinder(head_h, d=bolt_head_d, $fn=40);
}

module standoff()
{
	height = brd_h - brd_thick;
	
	difference() {
		cylinder(height, d1=bolt_d + 8, d2=bolt_d + 3, $fn=60);
		translate([0, 0, -0.1])
		bolt();
	}
}

boltoffs_x = [bolt_xoffs, brd_xsz - bolt_xoffs, bolt_xoffs, brd_xsz - bolt_xoffs];
boltoffs_y = [bolt_yoffs, bolt_yoffs, bolt_yoffs2, bolt_yoffs2];

module all_bolts()
{
	for(i=[0:3]) {
		translate([boltoffs_x[i], -boltoffs_y[i], -brd_h - thick])
		bolt();
	}
}

module box_bot()
{
	h = brd_h + thick;
	
	difference() {
		translate([-thick + bevel, -brd_ysz - thick + bevel, -h + bevel])
		minkowski() {
			cube([box_xsz - bevel * 2, box_ysz - bevel * 2, brd_h + thick]);
			sphere(bevel, $fn=50);
		}
		
		translate([0, -brd_ysz, -brd_h])
		cube([brd_xsz, brd_ysz, brd_h + 1]);
		
		translate([-box_xsz/2, -box_ysz*1.5, 0])
		cube([box_xsz * 2, box_ysz * 2, h]);
		
		din_conn();
		
		all_bolts();
	}
	
	standoff_y0 = -brd_h - thick + 0.01;

	for(i=[0:3]) {
		translate([boltoffs_x[i], -boltoffs_y[i], standoff_y0])
		standoff();
	}
}

module topshape(inset)
{
	h1 = brd_overhead_low + thick;
	h2 = brd_overhead_tall + thick;
	y0 = -bevel * 2;
	polygon([
		[thick - inset, y0],
		[thick - inset, h1 - inset],
		/*[-low_part, h1 - inset],
		[-low_part-3, h2 - inset],*/
		[-brd_ysz - thick + inset, h2 - inset],
		[-brd_ysz - thick + inset, y0]]);
}

module box_top()
{	
	h1 = brd_overhead_low + thick;
	h2 = brd_overhead_tall + thick;

	difference() {
		minkowski(convexity=10) {
			translate([-thick + bevel, 0, 0])
			rotate([90, 0, 90])
			linear_extrude(box_xsz - bevel * 2, convexity=10) {
				topshape(bevel);
			}
			sphere(r=bevel, $fn=50);
		}
		
		translate([0, 0, 0])
		rotate([90, 0, 90])
		linear_extrude(brd_xsz, convexity=10) {
			topshape(thick);
		}
		
		translate([-box_xsz/2, -box_ysz*1.5, -h2-1])
		cube([box_xsz * 2, box_ysz * 2, h2 + 1]);
		
		din_conn();
		
		top_holes();
		
		all_bolts();
	}
	
	difference() {
		foo_h = thick * 1.2;
		for(i=[0:3]) {
			translate([boltoffs_x[i], -boltoffs_y[i], brd_overhead_low - foo_h + 0.01])
			cylinder(foo_h, d1=bolt_head_d * 1.2, d2=bolt_head_d * 2, $fn=50);
		}
		
		all_bolts();
	}
}


//box_bot();

translate([0, 0, 5]) rotate([0, 0, 0]) box_top();