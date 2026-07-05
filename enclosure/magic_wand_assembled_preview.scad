// ============================================================
// AI Magic Wand — Gagang v6 (FIX: urutan operasi cincin & tab diperbaiki)
// STYLE = 0 -> Polos | 1 -> Elegan | 2 -> Kristal
//
// FIX dari v5: cincin dekoratif sekarang dipotong SEBELUM tab baut
// ditambahkan (bukan sesudah) — jadi tab tidak lagi ikut terbelah oleh
// alur cincin, benjolan baut jadi solid & rapi menyatu di posisi cincin.
//
// BAUT: sekrup mainan/RC self-tapping ~2mm x 6-8mm, dijual di toko mainan
// RC/elektronik/sparepart remote — tidak perlu mur.
// ============================================================

STYLE = 1;

grip_length      = 100;
wall_thickness   = 2.5;

usb_cutout_w     = 10;
usb_cutout_h     = 6;
usb_cutout_z     = 10;

switch_hole_d    = 6;
switch_z         = 30;

oled_window_w    = 26;
oled_window_h    = 15;
oled_window_z    = grip_length - 40;

grip_d_back      = 40;
grip_d_front     = 30;
ring_count       = 3;
ring_width       = 2;
ring_depth       = 0.8;
crystal_sides    = 6;

shaft_length     = 120;
shaft_d          = 14;
led_tip_d        = 10;

tab_w             = 5;
tab_len           = 5;
tab_h             = 5;
screw_hole_clear  = 2.3;
screw_hole_pilot  = 1.6;

screw_z_positions = (STYLE == 1)
    ? [for (i = [1:ring_count]) grip_length * i / (ring_count + 1)]
    : [15, 50, 85];

$fn = 48;

function outer_r(z) = (grip_d_back/2) + (grip_d_front/2 - grip_d_back/2) * (z / grip_length);

module tapered_body(d_back, d_front, len) {
    if (STYLE == 2) {
        linear_extrude(height = len, scale = d_front/d_back, $fn = crystal_sides)
            circle(d = d_back, $fn = crystal_sides);
    } else {
        cylinder(h = len, r1 = d_back/2, r2 = d_front/2);
    }
}

module tapered_cavity(d_back, d_front, len) {
    if (STYLE == 2) {
        linear_extrude(height = len, scale = (d_front - 2*wall_thickness)/(d_back - 2*wall_thickness), $fn = crystal_sides)
            circle(d = d_back - 2*wall_thickness, $fn = crystal_sides);
    } else {
        cylinder(h = len, r1 = d_back/2 - wall_thickness, r2 = d_front/2 - wall_thickness);
    }
}

module decorative_rings() {
    if (STYLE == 1) {
        for (i = [1 : ring_count]) {
            z_pos = grip_length * i / (ring_count + 1);
            r_here = outer_r(z_pos);
            translate([0, 0, z_pos])
                rotate_extrude($fn = $fn)
                    translate([r_here - ring_depth, 0, 0])
                        square([ring_depth + 1, ring_width], center = true);
        }
    }
}

module back_cap() {
    if (STYLE == 1) sphere(r = grip_d_back/2 * 0.3);
}

module screw_tab_bottom(z_pos) {
    r_here = outer_r(z_pos);
    translate([-tab_w/2, -(r_here + tab_len), z_pos - tab_h/2])
        cube([tab_w, tab_len + 0.05, tab_h]);
}

module screw_tab_top(z_pos) {
    r_here = outer_r(z_pos);
    translate([-tab_w/2, r_here - 0.05, z_pos - tab_h/2])
        cube([tab_w, tab_len + 0.05, tab_h]);
}

module screw_holes_pilot() {
    for (z_pos = screw_z_positions) {
        r_here = outer_r(z_pos);
        translate([0, -(r_here + tab_len + 1), z_pos])
            rotate([-90, 0, 0])
                cylinder(h = tab_len + 2, d = screw_hole_pilot);
    }
}

module screw_holes_clear() {
    for (z_pos = screw_z_positions) {
        r_here = outer_r(z_pos);
        translate([0, r_here - 1, z_pos])
            rotate([-90, 0, 0])
                cylinder(h = tab_len + 2, d = screw_hole_clear);
    }
}

// ---- Body dasar: silinder tirus + rongga + cincin SUDAH terpotong duluan ----
module ringed_hollow_body() {
    difference() {
        tapered_body(grip_d_back, grip_d_front, grip_length);
        translate([0, 0, wall_thickness])
            tapered_cavity(grip_d_back, grip_d_front, grip_length);
        decorative_rings();
    }
}

module grip_bottom() {
    difference() {
        union() {
            ringed_hollow_body();
            back_cap();
            for (z_pos = screw_z_positions) screw_tab_bottom(z_pos); // tab ditambah TERAKHIR, solid, tidak kepotong cincin
        }

        translate([-grip_d_back/2 - 1, 0, -grip_d_back/2])
            cube([grip_d_back + 2, grip_d_back + 2, grip_length + grip_d_back]);

        translate([-usb_cutout_w/2, -outer_r(usb_cutout_z) - 1, usb_cutout_z])
            cube([usb_cutout_w, outer_r(usb_cutout_z) + 2, usb_cutout_h]);

        translate([outer_r(switch_z) - wall_thickness - 2, 0, switch_z])
            rotate([0, 90, 0])
                cylinder(h = wall_thickness + 4, d = switch_hole_d);

        translate([-oled_window_w/2, -outer_r(oled_window_z) - 1, oled_window_z])
            cube([oled_window_w, outer_r(oled_window_z) + 2, oled_window_h]);

        screw_holes_pilot(); // lubang baut dibor terakhir, tembus tab yang sudah solid
    }
}

module grip_top() {
    difference() {
        union() {
            ringed_hollow_body();
            for (z_pos = screw_z_positions) screw_tab_top(z_pos);
        }

        translate([-grip_d_back/2 - 1, -grip_d_back - 2, -1])
            cube([grip_d_back + 2, grip_d_back + 2, grip_length + 2]);

        translate([-oled_window_w/2, 0, oled_window_z])
            cube([oled_window_w, outer_r(oled_window_z) + 2, oled_window_h]);

        screw_holes_clear();
    }
}

module wand_shaft() {
    difference() {
        if (STYLE == 2) {
            linear_extrude(height = shaft_length, scale = shaft_d/grip_d_front, $fn = crystal_sides)
                circle(d = grip_d_front, $fn = crystal_sides);
        } else {
            cylinder(h = shaft_length, r1 = grip_d_front/2, r2 = shaft_d/2);
        }
        translate([0, 0, -1])
            cylinder(h = shaft_length + 2, d = 4);
    }
    translate([0, 0, shaft_length])
        difference() {
            if (STYLE == 1) {
                sphere(r = shaft_d/2 + 1);
            } else {
                cylinder(h = 8, r1 = shaft_d/2, r2 = shaft_d/2 * 0.6, $fn = (STYLE == 2 ? crystal_sides : $fn));
            }
            translate([0, 0, 1]) cylinder(h = shaft_d, d = led_tip_d);
        }
}


// Preview MENYATU: shell bawah + atas digabung jadi 1 tabung utuh,
// batang depan disambung di ujung depan gagang
union() {
    grip_bottom();
    grip_top();
}
translate([0, 0, grip_length])
    wand_shaft();
