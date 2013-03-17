#ifndef ART_H
#define ART_H

const int FONT_SHEET_X_CELLS = 16;
const int FONT_SHEET_Y_CELLS = 7;
const int TILE_SHEET_X_CELLS = 19;
const int TILE_SHEET_Y_CELLS = 11;

enum Tile_t {
	tile_empty,
	tile_playerFirearm,
	tile_playerMelee,
	tile_zombieUnarmed,
	tile_zombieArmed,
	tile_zombieBloated,
	tile_cultistFirearm,
	tile_cultistDagger,
	tile_witchOrWarlock,
	tile_ghoul,
	tile_mummy,
	tile_deepOne,
	tile_shadow,
	tile_armor,
	tile_potion,
	tile_ammo,
	tile_scroll,
	tile_elderSign,
	tile_chestClosed,
	tile_chestOpen,
	tile_rat,
	tile_spider,
	tile_wolf,
	tile_phantasm,
	tile_ratThing,
	tile_hound,
	tile_bat,
	tile_byakhee,
	tile_massOfWorms,
	tile_ooze,
	tile_vortex,
	tile_ghost,
	tile_wraith,
	tile_mantis,
	tile_locust,
	tile_pistol,
	tile_tommyGun,
	tile_shotgun,
	tile_dynamite,
	tile_dynamiteLit,
	tile_molotov,
	tile_incinerator,
	tile_teslaCannon,
	tile_flare,
	tile_flareGun,
	tile_flareLit,
	tile_dagger,
	tile_axe,
	tile_club,
	tile_hammer,
	tile_machete,
	tile_pitchfork,
	tile_sledgeHammer,
	tile_rock,
	tile_ironSpike,
	tile_huntingHorror,
	tile_spiderLeng,
	tile_migo,
	tile_floor,
	tile_aimMarkerHead,
	tile_aimMarkerTrail,
	tile_wallTop,
	tile_wallFrontAlt1,
	tile_wallFrontAlt2,
	tile_wallFront,
	tile_squareCheckered,
	tile_rubbleHigh,
	tile_rubbleLow,
	tile_stairsDown,
  tile_leverLeft,
  tile_leverRight,
  tile_device1,
  tile_device2,
  tile_cabinetClosd,
  tile_cabinetOpen,
  tile_pillarBroken,
  tile_pillar,
  tile_pillarCarved,
  tile_barrel,
  tile_sarcophagus,
  tile_caveWallFront,
  tile_caveWallTop,
	tile_brazier,
	tile_altar,
	tile_spiderWeb,
	tile_doorClosed,
	tile_doorOpen,
	tile_doorBroken,
	tile_tree,
	tile_bush,
	tile_churchBench,
	tile_graveStone,
	tile_tomb,
	tile_water1,
	tile_water2,
	tile_trapGeneral,
	tile_cocoon,
	tile_blastAnimation1,
	tile_blastAnimation2,
	tile_corpse,
	tile_corpse2,
	tile_projectileStandardFrontSlash,
	tile_projectileStandardBackSlash,
	tile_projectileStandardDash,
	tile_projectileStandardVerticalBar,
	tile_gore1,
	tile_gore2,
	tile_gore3,
	tile_gore4,
	tile_gore5,
	tile_gore6,
	tile_gore7,
	tile_gore8,
	tile_smoke,
	tile_trapezohedron,
	tile_pit,
	tile_popupCornerTopLeft,
	tile_popupCornerTopRight,
	tile_popupCornerBottomLeft,
	tile_popupCornerBottomRight,
	tile_popupHorizontalBar,
	tile_popupVerticalBar
};

class Engine;
struct coord;

class Art {
public:
	coord getGlyphCoords(const char glyph);
	coord getTileCoords(const Tile_t tile);

private:
	coord getColumnAndRow(const char glyph);
	coord getColumnAndRowTile(const Tile_t tile);
};

#endif
