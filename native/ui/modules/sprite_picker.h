#pragma once
#include <algorithm>
#include <string>
#include "ui/helpers.h"
#include "ui/sprite_renderer.h"
namespace pt::ui {
struct SpritePickerState { bool open=false; int instrument=0; int selection=0; };
inline int sprite_picker_sprite_id(const SpritePickerState& s) { return s.selection<=0 ? -1 : std::clamp(s.selection-1,0,SPRITE_COUNT-1); }
inline void sprite_picker_move(SpritePickerState& s,int d){ if(!s.open)return; s.selection=(s.selection+d)%(SPRITE_COUNT+1); if(s.selection<0)s.selection+=SPRITE_COUNT+1; }
inline void sprite_picker_random(SpritePickerState& s,uint32_t seed){ uint32_t x=seed?seed:0x9E3779B9u; x^=x<<13; x^=x>>17; x^=x<<5; s.selection=1+static_cast<int>(x%SPRITE_COUNT); }
inline void draw_sprite_picker(Canvas& c,const SpritePickerState& s,const Theme& t){
 if(!s.open)return; draw_modal_backdrop(c); constexpr int W=330,H=285,X=(DESIGN_W-W)/2,Y=(DESIGN_H-H)/2,RX=X+24,RY=Y+62,RW=282,RH=64,CX=RX+(RW-32)/2,CY=RY+16;
 draw_modal_box(c,X,Y,W,H,t); c.draw_text("SPRITE PICKER",X+46,Y+12,t.textTitle,CHAR_SPACING,FONT_SCALE); c.stroke_rect(RX,RY,RW,RH,t.textParam);
 const int id=sprite_picker_sprite_id(s), prev=(id<0?SPRITE_COUNT-1:(id+SPRITE_COUNT-1)%SPRITE_COUNT), next=(id<0?0:(id+1)%SPRITE_COUNT);
 if(id>=0) draw_sprite_clipped(c,id,CX,CY,RX,RY,RW,RH); else c.draw_text("NO SPRITE",RX+86,CY+8,t.textEmpty,CHAR_SPACING,FONT_SCALE);
 draw_sprite_clipped(c,prev,CX,CY-32,RX,RY,RW,RH); draw_sprite_clipped(c,next,CX,CY+32,RX,RY,RW,RH);
 const std::string name=id<0?"NO SPRITE":SPRITE_NAMES[id]; const std::string num=id<0?"-- / 128":hex2(id)+" / 128";
 c.draw_text(name,X+24,Y+145,t.textValue,CHAR_SPACING,FONT_SCALE); c.draw_text(num,X+230,Y+145,t.textParam,CHAR_SPACING,FONT_SCALE);
 c.draw_text("A+UP/DN  SCROLL",X+24,Y+178,t.textParam,CHAR_SPACING,FONT_SCALE); c.draw_text("A+B      SPIN",X+24,Y+198,t.textParam,CHAR_SPACING,FONT_SCALE); c.draw_text("B        ASSIGN",X+24,Y+218,t.textValue,CHAR_SPACING,FONT_SCALE); c.draw_text("L        CANCEL",X+24,Y+238,t.textParam,CHAR_SPACING,FONT_SCALE);
}
}
