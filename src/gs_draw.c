#include "gs_draw.h"

#include <string.h>

#include "camera_util.h"

void GsDrawCheckerboard(GameScene* GS) {
  const int check_size = 128;
  const fixed_t check_z = default_z;

  fixed_t check_center_x = (uint32_t)GS->player.x / FixNew(check_size, 0) / 2 * FixNew(check_size, 0) * 2;
  fixed_t check_center_y = (uint32_t)GS->player.y / FixNew(check_size, 0) / 2 * FixNew(check_size, 0) * 2;

  for (int yidx = -4; yidx <= 3; ++yidx) {
    for (int xidx = -3; xidx <= 2; ++xidx) {
      fixed_t check_x = check_center_x - (xidx * FixNew(check_size, 0) * 2);
      fixed_t check_y = check_center_y - (yidx * FixNew(check_size, 0));
      if (yidx % 2 == 0) {
        check_x -= FixNew(check_size, 0);
      }

      int render_x, render_y;
      GetRenderCoords(GS, check_x, check_y, check_z, &render_x, &render_y);

      int render_check_size = GetRenderLength(GS, check_size, check_z);

      DrawRectangle(render_x, render_y, render_check_size, render_check_size, LIGHTGRAY);
    }
  }
}

void GsDrawProjs(GameScene* GS) {
  for (int p = 0; p < LENGTHOF(GS->projs); ++p) {
    if (!GS->projs[p].exists) {
      continue;
    }
    int rx, ry;
    GetRenderCoords(GS, GS->projs[p].x, GS->projs[p].y, default_z, &rx, &ry);
    Vector2 render_pos = {rx, ry};
    int render_size = GetRenderLength(GS, GS->projs[p].size, default_z);
    IntClamp(&render_size, 4, 100);

    Color color = GRAY;
    if (GS->projs[p].homing_power > 0) {
      if (GS->projs[p].is_homing && GS->projs[p].despawn_timer % 2 == 0) {
        color = WHITE;
      } else {
        color = MAGENTA;
      }
    }
    // if both flaming and frost, alternate blue and orange
    if (GS->projs[p].frost_power > 0 && GS->projs[p].flame_power > 0) {
      if (GS->projs[p].despawn_timer % 2 == 0) {
        color = SKYBLUE;
      } else {
        color = ORANGE;
      }
    } else if (GS->projs[p].frost_power > 0) {
      color = SKYBLUE;
    } else if (GS->projs[p].flame_power > 0) {
      color = ORANGE;
    }

    int sides = 4;
    if (GS->projs[p].frost_power > 0) {
      sides = 10;
    }
    if (GS->projs[p].split_fragments > 0) {
      sides = 5;
    }
    if (GS->projs[p].is_spike) {
      sides = 3;
    }
    DrawPoly(render_pos, sides, render_size, GS->ticks + p * 15, color);
    DrawPolyLinesEx(render_pos, sides, render_size, GS->ticks + p * 15, 2.0f, BLACK);

    // debug info
    // DrawLine(render_pos.x, render_pos.y,
    //          render_pos.x + FixWhole(FixCos(GS->projs[p].move_angle) * GS->projs[p].move_speed * target_fps / fixed_factor / 4),
    //          render_pos.y + FixWhole(FixSin(GS->projs[p].move_angle) * GS->projs[p].move_speed * target_fps / fixed_factor / 4),
    //          RED);
    // DrawLine(render_pos.x, render_pos.y,
    //          render_pos.x + FixWhole(FixCos(GS->projs[p].orbit.angle) * 16),
    //          render_pos.y + FixWhole(FixSin(GS->projs[p].orbit.angle) * 16),
    //          BLUE);
  }
}

void GsDrawShapes(GameScene* GS) {
  for (int i = 0; i < LENGTHOF(GS->shapes); ++i) {
    if (!GS->shapes[i].exists) {
      continue;
    }
    int rx, ry;
    GetRenderCoords(GS, GS->shapes[i].x, GS->shapes[i].y, default_z, &rx, &ry);
    Vector2 render_pos = {rx, ry};

    // frost or flame background
    if (GS->shapes[i].frost_ticks > 0 || GS->shapes[i].flame_ticks > 0) {
      bool show_frost = GS->shapes[i].frost_ticks > 0;
      bool show_flame = GS->shapes[i].flame_ticks > 0;
      Color colors[2];
      if (show_frost && show_flame) {
        colors[0] = SKYBLUE;
        colors[1] = ORANGE;
      } else if (show_frost) {
        colors[0] = SKYBLUE;
        colors[1] = SKYBLUE;
      } else {
        colors[0] = ORANGE;
        colors[1] = ORANGE;
      }

      if (show_frost || show_flame) {
        // draw
        int fxsize = GetRenderLength(GS, GS->shapes[i].size - 6 + IntMin(10, GS->shapes[i].frost_ticks / 4) + IntMin(10, GS->shapes[i].flame_ticks / 4), default_z);
        DrawPoly(render_pos, 3, fxsize, GS->ticks * 4, colors[0]);
        DrawPoly(render_pos, 3, fxsize, -GS->ticks * 4, colors[1]);
      }
    }

    // shape
    Color fg = (GS->shapes[i].ticks_since_damaged >= 8) ? GS->shapes[i].fg : WHITE;
    Color bg = (GS->shapes[i].ticks_since_damaged >= 8) ? GS->shapes[i].bg : PINK;
    int rotation = GS->ticks;
    if (GS->shapes[i].variant == SHAPE_VARIANT_FAST) {
      rotation = GS->ticks * 3;
    }
    if (GS->shapes[i].variant == SHAPE_VARIANT_HEALING) {
      rotation = -GS->ticks;
    }
    if (GS->shapes[i].frost_ticks > 0) {
      rotation = 0;
      fg = WHITE;
      // render_pos.x += GetRandomValue(0, 1);
      render_pos.x += GS->shapes[i].frost_ticks % 2;
      render_pos.y += GS->shapes[i].frost_ticks % 2;
    }
    int render_size = GetRenderLength(GS, GS->shapes[i].size, default_z);
    if (render_size < 3) {
      render_size = 3;
    }
    DrawPoly(render_pos, GS->shapes[i].sides, render_size, rotation, fg);
    DrawPolyLinesEx(render_pos, GS->shapes[i].sides, render_size, rotation, 2.0f, bg);

    // healthbar
    if (GS->shapes[i].hp < GS->shapes[i].max_hp) {
      int bar_width = IntMin(render_w / 2, GetRenderLength(GS, GS->shapes[i].max_hp / 20, default_z));
      int filled_width = bar_width * GS->shapes[i].hp / GS->shapes[i].max_hp;
      Color color = GREEN;
      if (filled_width <= bar_width / 2) {
        color = YELLOW;
      }
      if (filled_width <= bar_width / 4) {
        color = RED;
      }
      DrawRectangle(render_pos.x - bar_width / 2, render_pos.y + render_size + 2, bar_width, 3, BLACK);
      DrawRectangle(render_pos.x - bar_width / 2, render_pos.y + render_size + 2, filled_width, 3, color);
    }

    // debug info
    // DrawPrintf(render_pos.x, render_pos.y, BLACK, "[%d]", i);
    // DrawLine(render_pos.x, render_pos.y,
    //          render_pos.x + FixWhole(FixCos(GS->shapes[i].move_angle) * GS->shapes[i].move_speed * target_fps / fixed_factor),
    //          render_pos.y + FixWhole(FixSin(GS->shapes[i].move_angle) * GS->shapes[i].move_speed * target_fps / fixed_factor),
    //          RED);
    // DrawLine(render_pos.x, render_pos.y,
    //          render_pos.x + FixWhole(FixCos(GS->shapes[i].kb_angle) * GS->shapes[i].kb_speed * target_fps / fixed_factor),
    //          render_pos.y + FixWhole(FixSin(GS->shapes[i].kb_angle) * GS->shapes[i].kb_speed * target_fps / fixed_factor),
    //          BLUE);
  }
}

// hack
int GsXpForLevelUp(GameScene* GS);

void GsDrawPlayer(GameScene* GS) {
  int rx, ry;
  GetRenderCoords(GS, GS->player.x, GS->player.y, default_z, &rx, &ry);
  Vector2 render_pos = {rx, ry};
  int cannon_distance = GS->player.stats.size * 3 / 4 + GS->player.ticks_since_last_shot;
  IntClamp(&cannon_distance, GS->player.stats.size * 3 / 4, GS->player.stats.size * 5 / 4);
  GetRenderCoords(GS,
                  GS->player.x + FixCos(GS->player.angle) * cannon_distance,
                  GS->player.y + FixSin(GS->player.angle) * cannon_distance,
                  default_z,
                  &rx, &ry);
  Vector2 cannon_pos = {rx, ry};
  Color fg = (GS->player.ticks_since_damaged <= 30 && GS->player.ticks_since_damaged % 4 < 2) ? WHITE : GRAY;
  Color bg = (GS->player.ticks_since_damaged <= 30 && GS->player.ticks_since_damaged % 4 < 2) ? PINK : BLACK;

  DrawPoly(cannon_pos, 4, GetRenderLength(GS, GS->player.stats.size * 3 / 5, default_z), (32 + GS->player.angle) * 360 / angle_factor, fg);
  DrawPolyLinesEx(cannon_pos, 4, GetRenderLength(GS, GS->player.stats.size * 3 / 5, default_z), (32 + GS->player.angle) * 360 / angle_factor, 2.5f, bg);

  DrawPoly(render_pos, 20, GetRenderLength(GS, GS->player.stats.size, default_z), 0, fg);
  DrawPolyLinesEx(render_pos, 20, GetRenderLength(GS, GS->player.stats.size, default_z), 0, 2.0f, bg);

  // DrawLine(render_pos.x,
  //          render_pos.y,
  //          render_pos.x + FixWhole(FixCos(GS->player.angle) * 16),
  //          render_pos.y + FixWhole(FixSin(GS->player.angle) * 16),
  //          GREEN);

  // healthbar
  if (GS->player.hp < GS->player.stats.max_hp) {
    int bar_width = GetRenderLength(GS, GS->player.stats.max_hp / 20, default_z);
    int filled_width = bar_width * GS->player.hp / GS->player.stats.max_hp;
    Color color = GREEN;
    if (filled_width <= bar_width / 2) {
      color = YELLOW;
    }
    if (filled_width <= bar_width / 4) {
      color = RED;
    }
    DrawRectangle(render_pos.x - bar_width / 2 - 1, render_pos.y + GS->player.stats.size + 2 - 1, bar_width + 2, 5, BLACK);
    DrawRectangle(render_pos.x - bar_width / 2, render_pos.y + GS->player.stats.size + 2, filled_width, 3, color);
  }

  // magnetism
  if (GS->player.ticks_since_magnetism < 2) {
    DrawPolyLinesEx(render_pos, 8, GetRenderLength(GS, GS->player.stats.magnetism_dist, default_z), 22.5f, 2.0f, GREEN);
  }
}

void GsDrawPickups(GameScene* GS) {
  for (int p = 0; p < LENGTHOF(GS->pickups); ++p) {
    if (!GS->pickups[p].exists) {
      continue;
    }
    int rx, ry;
    GetRenderCoords(GS, GS->pickups[p].x, GS->pickups[p].y, default_z, &rx, &ry);
    Vector2 render_pos = {rx, ry};
    int render_size = GetRenderLength(GS, 12, default_z);
    if (render_size < 6) {
      render_size = 6;
    }

    DrawPoly(render_pos, 4, render_size, 0, WHITE);
    DrawPolyLinesEx(render_pos, 4, render_size, 0, 2.0f, SKYBLUE);
    DrawPrintf(render_pos.x - 3 * strlen(item_strs[GS->pickups[p].type]), render_pos.y, BLACK, "%s", item_strs[GS->pickups[p].type]);
    if (FixAbs(GS->pickups[p].x - GS->player.x) > FixNew(render_w / 2, 0) ||
        FixAbs(GS->pickups[p].y - GS->player.y) > FixNew(render_h / 2, 0)) {
      Vector2 marker_pos = {FixWhole(GS->player.x - GS->camera.x) + render_w / 2,
                            FixWhole(GS->player.y - GS->camera.y) + render_h / 2};
      marker_pos.x += FixWhole(FixCos(GS->pickups[p].angle_to_player) * -150);
      marker_pos.y += FixWhole(FixSin(GS->pickups[p].angle_to_player) * -110);
      DrawPrintf(marker_pos.x, marker_pos.y, BLACK, "!");
    }
  }
}

// wip
void GsDrawLineFx(GameScene* GS) {
  for (int t = 0; t < LENGTHOF(GS->line_fx); ++t) {
    if (!GS->line_fx[t].exists) {
      continue;
    }
    int rx1, ry1;
    GetRenderCoords(GS, GS->line_fx[t].x1, GS->line_fx[t].y1, default_z, &rx1, &ry1);

    int rx2, ry2;
    GetRenderCoords(GS, GS->line_fx[t].x2, GS->line_fx[t].y2, default_z, &rx2, &ry2);

    DrawLine(rx1, ry1, rx2, ry2, GS->line_fx[t].color);
  }
}

void GsDrawTextFx(GameScene* GS) {
  for (int t = 0; t < LENGTHOF(GS->text_fx); ++t) {
    if (!GS->text_fx[t].exists) {
      continue;
    }
    int rx, ry;
    GetRenderCoords(GS, GS->text_fx[t].x, GS->text_fx[t].y, default_z, &rx, &ry);
    Vector2 render_pos = {rx, ry};
    DrawPrintf(render_pos.x - 3 * strlen(GS->text_fx[t].text), render_pos.y, GS->text_fx[t].color, "%s", GS->text_fx[t].text);
  }
}

void GsDrawShapeFx(GameScene* GS) {
  for (int s = 0; s < LENGTHOF(GS->shape_fx); ++s) {
    if (!GS->shape_fx[s].exists) {
      continue;
    }
    int rx, ry;
    GetRenderCoords(GS, GS->shape_fx[s].x, GS->shape_fx[s].y, default_z, &rx, &ry);
    Vector2 render_pos = {rx, ry};

    // shape

    int render_size = GetRenderLength(GS, GS->shape_fx[s].size, default_z);
    if (render_size < 3) {
      render_size = 3;
    }
    DrawPoly(render_pos, GS->shape_fx[s].sides, render_size, GS->shape_fx[s].angle, GS->shape_fx[s].fg);
    DrawPolyLinesEx(render_pos, GS->shape_fx[s].sides, render_size, GS->shape_fx[s].angle, 2.0f, GS->shape_fx[s].bg);

    // turn or grow the shape_fx
    GS->shape_fx[s].angle += GS->shape_fx[s].rotation_amount;
    GS->shape_fx[s].size += GS->shape_fx[s].grow_amount;
  }
}

void GsDrawXpOrbs(GameScene* GS) {
  for (int o = 0; o < LENGTHOF(GS->xp_orbs); ++o) {
    if (!GS->xp_orbs[o].exists) {
      continue;
    }
    int rx, ry;
    GetRenderCoords(GS, GS->xp_orbs[o].x, GS->xp_orbs[o].y, default_z, &rx, &ry);
    Vector2 render_pos = {rx, ry};
    Color colors[] = {GREEN, LIME, GREEN, GREEN, LIME, GREEN};
    int size = GetRenderLength(GS, GS->xp_orbs[o].xp / 2, default_z);
    DrawCircleV(render_pos, ((size < 2) ? 2 : size), colors[GS->xp_orbs[o].age % LENGTHOF(colors)]);
  }
}

void GsDrawUi(GameScene* GS) {
  {
    int bar_width = render_w;
    int target_filled_width = bar_width * GS->player.xp / GsXpForLevelUp(GS);
    FixLerp(&GS->draw_data.xp_bar_filled_width, FixNew(target_filled_width, 0), FixNew(0, 32));
    Color color = MAGENTA;
    DrawRectangle(0, render_h - 4, bar_width, 4, BLACK);
    DrawRectangle(0, render_h - 4, FixWhole(GS->draw_data.xp_bar_filled_width), 4, color);
    DrawPrintf(0, render_h - 4 - ft_height, BLACK, "Level %d, %d/%d XP", GS->player.level, GS->player.xp, GsXpForLevelUp(GS));
    if (GS->player.upgrades_pending > 0) {
      DrawPrintf(0, render_h - 4 - ft_height * 2, DARKBLUE, "Press [SPACE] to receive %d upgrade%s!",
                 GS->player.upgrades_pending, GS->player.upgrades_pending == 1 ? "" : "s");
    }
  }
}

void GsDrawOlPickItem(GameScene* GS) {
  const int ol_x = render_w / 12;
  const int ol_y = render_h / 12;
  const int ol_w = render_w * 10 / 12;
  const int ol_h = render_h * 10 / 12;
  const int lh = ft_height;  // line height
  DrawRectangle(ol_x, ol_y, ol_w, ol_h, BLACK);

  DrawPrintf(ol_x + lh, ol_y + lh, YELLOW, "Pick an upgrade!");
  DrawPrintf(ol_x + lh, ol_y + lh + lh, GRAY, "Upgrades remaining: %d", GS->player.upgrades_pending);

  for (int i = 0; i < GS->ol_pick_item.item_count; ++i) {
    int text_x = ol_x + lh;
    int text_y = ol_y + lh * (4 + i * 4);

    bool is_selected = (i == GS->ol_pick_item.selected_item_idx);
    const char* prefix = (is_selected ? "> " : " ");

    char buf[256] = "";
    strcpy(buf, prefix);
    strcat(buf, "+");
    strcat(buf, item_strs[GS->ol_pick_item.choices[i].item_a]);
    if (GS->ol_pick_item.choices[i].item_b != ITEM_INVALID) {
      strcat(buf, "\n");
      strcat(buf, prefix);
      strcat(buf, "+");
      strcat(buf, item_strs[GS->ol_pick_item.choices[i].item_b]);
    }
    if (GS->ol_pick_item.choices[i].removed_item != ITEM_INVALID) {
      strcat(buf, "\n");
      strcat(buf, prefix);
      strcat(buf, "-");
      strcat(buf, item_strs[GS->ol_pick_item.choices[i].removed_item]);
    }
    DrawPrintf(text_x, text_y, WHITE, "%s", buf);
  }

  // Draw stat changes
  {
    int text_x = render_w / 2;
    int text_y = ol_y + lh * 4;
    for (GsPlayerStatType s = 0; s < STAT_COUNT; ++s) {
      if (GS->player.stats.as_int[s] != GS->player.tmp_stats.as_int[s]) {
        bool is_upgrade = stat_lower_is_better[s] == (GS->player.tmp_stats.as_int[s] <= GS->player.stats.as_int[s]);
        DrawPrintf(text_x, text_y, (is_upgrade ? LIME : RED),
                   "%s:\n  %d -> %d",
                   stat_names[s],
                   GS->player.stats.as_int[s],
                   GS->player.tmp_stats.as_int[s]);
        text_y += lh * 3;
      }
    }
  }

  DrawPrintf(ol_x + lh, ol_y + ol_h - lh * 3, GRAY, "W/S to select, \nSPACE to confirm");
}

void GsDrawOlPauseText(GameScene* GS) {
  const char* title_text = "GAME PAUSED";
  const char* tabs_text = "STATS         SETTINGS         ITEMS";
  const char* stats_arrows = "^^^^^                               ";
  const char* settings_arrows = "              ^^^^^^^^              ";
  const char* items_arrows = "                               ^^^^^";
  const int ol_x = render_w * 1 / 12;
  const int ol_y = render_h * 1 / 12;
  const int ol_w = render_w * 10 / 12;
  const int ol_h = render_h * 3 / 12;

  DrawRectangle(ol_x, ol_y, ol_w, ol_h, GRAY);
  DrawRectangle(ol_x + 3, ol_y + 3, ol_w - 6, ol_h - 6, LIGHTGRAY);

  DrawPrintf(render_w / 2 - (strlen(title_text) * ft_width / 2), ol_y + ft_height, BLACK, "%s", title_text);
  DrawPrintf(render_w / 2 - (strlen(tabs_text) * ft_width / 2), ol_y + ft_height * 3, BLACK, "%s", tabs_text);
  const char* arrows_to_print = stats_arrows;
  if (GS->curr_overlay == GS_OVERLAY_SETTINGS) {
    arrows_to_print = settings_arrows;
  } else if (GS->curr_overlay == GS_OVERLAY_ITEMS) {
    arrows_to_print = items_arrows;
  }
  DrawPrintf(render_w / 2 - (strlen(arrows_to_print) * ft_width / 2), ol_y + ft_height * 4, BLACK, "%s", arrows_to_print);
}

void GsDrawOlStats(GameScene* GS) {
  const char* stats_title_text = "STATS";
  const int ol_x = render_w * 1 / 12;
  const int ol_y = render_h * 3 / 12;
  const int ol_w = render_w * 10 / 12;
  const int ol_h = render_h * 8 / 12;
  const int lh = ft_height;  // line height
  DrawRectangle(ol_x, ol_y, ol_w, ol_h, BLUE);
  DrawRectangle(ol_x + 3, ol_y + 3, ol_w - 6, ol_h - 6, SKYBLUE);
  DrawPrintf(render_w / 2 - (strlen(stats_title_text) * ft_width / 2), ol_y + lh, BLACK, "%s", stats_title_text);

  // start printing on the left side, and move to the right side when we run out of room.
  int stats_printed = 0;
  bool right_side = false;
  int l = 3;
  for (int i = 0; i < STAT_COUNT; ++i) {
    if (GS->player.stats.as_int[i] == 0) {
      continue;  // Don't print stats equal to 0 to reduce clutter.
    }
    int x = (right_side) ? (render_w / 2) : (ol_x + lh);
    int y = ft_height * l + ol_y;
    DrawPrintf(x, y, BLACK, "%s: %d", stat_names[i], GS->player.stats.as_int[i]);
    ++l;
    ++stats_printed;
    if (stats_printed == 15) {
      right_side = true;
      l = 3;
    }
  }
}

void GsDrawOlSettings(GameScene* GS) {
  const char* settings_title_text = "SETTINGS";
  const int ol_x = render_w * 1 / 12;
  const int ol_y = render_h * 3 / 12;
  const int ol_w = render_w * 10 / 12;
  const int ol_h = render_h * 8 / 12;
  const int lh = ft_height;  // line height
  DrawRectangle(ol_x, ol_y, ol_w, ol_h, DARKGRAY);
  DrawRectangle(ol_x + 3, ol_y + 3, ol_w - 6, ol_h - 6, GRAY);
  DrawPrintf(render_w / 2 - (strlen(settings_title_text) * ft_width / 2), ol_y + lh, BLACK, "%s", settings_title_text);
}

void GsDrawOlItems(GameScene* GS) {
  const char* items_title_text = "ITEMS";
  const int ol_x = render_w * 1 / 12;
  const int ol_y = render_h * 3 / 12;
  const int ol_w = render_w * 10 / 12;
  const int ol_h = render_h * 8 / 12;
  const int lh = ft_height;  // line height
  DrawRectangle(ol_x, ol_y, ol_w, ol_h, BROWN);
  DrawRectangle(ol_x + 3, ol_y + 3, ol_w - 6, ol_h - 6, BEIGE);
  DrawPrintf(render_w / 2 - (strlen(items_title_text) * ft_width / 2), ol_y + lh, BLACK, "%s", items_title_text);

  // Start printing on the left side, and move to the right side when we run out of room.
  int items_printed = 0;
  bool right_side = false;
  int l = 3;
  for (int i = 0; i < ITEM_COUNT; ++i) {
    if (GS->player.item_counts[i] == 0) {
      continue;  // Don't print items equal to 0 to reduce clutter.
    }
    int x = (right_side) ? (render_w / 2) : (ol_x + lh);
    int y = ft_height * l + ol_y;
    if (GS->player.item_counts[i] == 1) {
      DrawPrintf(x, y, BLACK, "%s", item_strs[i]);
    } else {
      DrawPrintf(x, y, BLACK, "%s (x%d)", item_strs[i], GS->player.item_counts[i]);
    }
    ++l;
    ++items_printed;
    if (items_printed == 15) {
      right_side = true;
      l = 3;
    }
  }
}

void GsDraw(GameScene* GS) {
  // Always draw the main screen.
  GsDrawCheckerboard(GS);
  GsDrawProjs(GS);
  GsDrawShapes(GS);
  GsDrawPlayer(GS);
  GsDrawPickups(GS);
  GsDrawXpOrbs(GS);
  GsDrawUi(GS);
  GsDrawLineFx(GS);
  GsDrawTextFx(GS);
  GsDrawShapeFx(GS);

  switch (GS->curr_overlay) {
    case GS_OVERLAY_NONE: {
    } break;

    case GS_OVERLAY_PICK_ITEM: {
      GsDrawOlPickItem(GS);
    } break;

    case GS_OVERLAY_STATS: {
      GsDrawOlPauseText(GS);
      GsDrawOlStats(GS);
    } break;

    case GS_OVERLAY_SETTINGS: {
      GsDrawOlPauseText(GS);
      GsDrawOlSettings(GS);
    } break;

    case GS_OVERLAY_ITEMS: {
      GsDrawOlPauseText(GS);
      GsDrawOlItems(GS);
    } break;

    default: {
      TraceLog(LOG_ERROR, "Bad overlay value %d", GS->curr_overlay);
    } break;
  }
}