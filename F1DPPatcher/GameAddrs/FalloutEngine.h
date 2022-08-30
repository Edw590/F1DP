// sfall
// Copyright (C) 2008-2016 The sfall team, 2022 DADi590
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

// Original code modified by me, DADi590, to adapt it to this project, starting on 2022-03-02.
// NOTE: I don't see mention to Crafty in the copyright notices, but I'll just say here that this code was taken from
// his modification of the original sFall1.

#ifndef F1DPPATCHER_FALLOUTENGINE_H
#define F1DPPATCHER_FALLOUTENGINE_H



#include <stdint.h>

// The Xs are so that if a macro that doesn't have an address on it yet is used, an error will be thrown instantly.
// When putting the address, just add the 0 before the X (which I just took out for the errors to appear) and that's it.

// C_-started constants are for code section addresses, and D_-started constants are for data section addresses. This is
// so that on code, it's easy to know to which segment the macro belongs - just look at the prefix.

// Also, ALL function macros MUST have a comment on them saying if the arguments are passed on registers, registers +
// on the stack, or only on the stack, or if it has no args at all, but as it's not easy to know that, if there are no
// PUSHes for the function, just say it's registers - this is not to know how to use the function, it's just to know if
// one can use PUSHes and POPs near the function or not (only if the function doesn't have arguments on the stack).
//
// WARNING: I'm finding pointers to functions to be on the Data section too!!! Be careful with that.

// Things to note in each macro:
// - "Args: X", where X is "+stack" (indicating "at least stack"), or "non-stack" (indicating "no stack parameters",
//    meaning that IF they exist, they're on registers - it's not trivial(/easy?) to check if there are register
//    parameters as it is with stack parameters, and it's not needed for the usage here, so this is enough)
// - After that, a forward slash "/" and the function prototype written on IDA. If there's no prototype and if the
//   function requires stack parameters, write how many parameters they are (check on IDA). If it's a pointer to a
//   function and there's no prototype, I think one must assume it can point to different functions at different parts
//   of the program, so don't put the number of stack parameters there. Those cases must be analyzed on a case-by-case
//   basis (:suicide:).
// - If there is no prototype, write "No prototype" after the forward slash.
// - And also if there's no prototype, check if the function cleans up the stack at exit or not, and if it does, write
//   "(__cdecl)" before the first forward slash.
//
// Why have the prototype if we don't want to know how to use the function? Because it says the calling convention,
// which is useful to know sometimes (like with a function which I think has the prototype calling convention written
// wrong, but the number of parameters are there - sum up: was useful, so now I'm doing that always).

// //////////////////////////////////////////////////////////////////////////////
// CODE SECTION HERE

#define C_action_get_an_object_              0x11F98 // Args: non-stack / No prototype
#define C_action_loot_container_             0x12254 // Args: non-stack / No prototype
#define C_action_use_an_item_on_object_      0x11DA8 // Args: non-stack / int __usercall action_use_an_item_on_object_@<eax>(pobj *source@<eax>, pobj *target@<edx>, iobj *item@<ebx>)
#define C_AddHotLines_                       0x891DC // Args: non-stack / No prototype
#define C_adjust_ac_                         0x98FA4 // Args: non-stack / int __usercall adjust_ac_@<eax>(pobj *source@<eax>, iobj *newarmor@<ebx>, iobj *oldarmor@<edx>)
#define C_adjust_fid_                        x
#define C_art_alias_num_                     x
#define C_art_exists_                        0x19064 // Args: non-stack / _BOOL4 __usercall art_exists_@<eax>(DWORD fid@<eax>)
#define C_art_flush_                         x
#define C_art_frame_data_                    x
#define C_art_frame_length_                  x
#define C_art_frame_width_                   x
#define C_art_id_                            0x193FC // Args: +stack (auto-clean) / int __userpurge art_id_@<eax>(int ObjType@<eax>, int Index@<edx>, int ID1@<ecx>, int ID2@<ebx>, int ID3)
#define C_art_ptr_lock_                      x
#define C_art_ptr_lock_data_                 0x18968 // Args: non-stack / No prototype
#define C_art_ptr_unlock_                    0x18A5C // Args: non-stack / int __usercall art_ptr_unlock_@<eax>(int art_ptr@<eax>)
#define C_buf_fill_                          x
#define C_buf_to_buf_                        0xBD6F4 // Args: +stack (no-cleaning) / int __cdecl buf_to_buf_(int from, int width, int height, int from_width, int to, int to_width)
#define C_closedir_                          x
#define C_combat_ai_                         0x25E4C // Args: non-stack / No prototype
#define C_combat_anim_finished_              0x236A4 // Args: non-stack / No prototype
#define C_combat_is_shot_blocked_            0x244C8 // +stack (auto-clean) / int __userpurge combat_is_shot_blocked_@<eax>(pobj *source@<eax>, int source_tile@<edx>, pobj *target@<ecx>, int target_tile@<ebx>, int accumulator)
#define C_combat_should_end_                 0x20B84 // Args: non-stack / No prototype
#define C_combat_turn_                       0x208C8 // Args: non-stack / signed int __usercall combat_turn_@<eax>(pobj *source@<eax>, _BOOL4 dude_turn@<edx>)
#define C_config_set_value_                  x
#define C_container_exit_                    0x69250 // Args: non-stack / No prototype
#define C_credits_                           0x27200 // Args: non-stack / No prototype
#define C_credits_get_next_line_             0x27998 // Args: non-stack / No prototype
#define C_critter_body_type_                 0x28778 // Args: non-stack / int __usercall critter_body_type_@<eax>(pobj *source@<eax>)
#define C_critter_is_dead_                   0x286CC // Args: non-stack / No prototype
#define C_critter_kill_                      0x28440 // Args: non-stack / void __usercall critter_kill_(pobj *source@<eax>, _BOOL4 refresh@<ebx>, int death_frame@<edx>)
#define C_critter_name_                      0x27BD8 // Args: non-stack / No prototype
#define C_critter_pc_set_name_               0x27C68 // Args: non-stack / No prototype
#define C_db_close_                          x
#define C_db_create_database_                x
#define C_db_current_                        x
#define C_db_dir_entry_                      0xAF168 // Args: non-stack / signed int __usercall db_dir_entry_@<eax>(char *filename@<eax>, DWORD *filesize@<edx>)
#define C_db_fclose_                         x
#define C_db_freadIntCount_                  x
#define C_db_fwriteIntCount_                 x
#define C_db_init_database_                  x
#define C_db_init_hash_table_                x
#define C_db_init_patches_                   x
#define C_db_select_                         x
#define C_debug_register_env_                0xB3080 // Args: non-stack / No prototype
#define C_dialog_out_                        0x1BFA0 // Args: +stack (auto-clean) / No prototype (5 stack arguments)
#define C_display_inventory_                 0x639E8 // Args: non-stack / int __usercall display_inventory_@<eax>(int inventory_offset@<eax>, int mode@<ebx>, int visible_offset@<edx>)
#define C_display_print_                     0x2C05C // Args: non-stack / No prototype
#define C_display_scroll_down_               x
#define C_display_scroll_up_                 x
#define C_display_stats_                     0x654F4 // Args: non-stack / No prototype
#define C_display_table_inventories_         0x683C8 // Args: non-stack / int __usercall display_table_inventories_@<eax>(int GNWID@<eax>, pobj *target@<ebx>, int a3@<ecx>, pobj *source@<edx>)
#define C_display_target_inventory_          0x63E90 // Args: non-stack / void __usercall display_target_inventory_(int inventory_offset@<eax>, int visible_offset@<edx>, Inventory *target_inventory@<ebx>, int mode@<ecx>)
#define C_DOSCmdLineDestroy_                 x
#define C_draw_box_                          0xBD1C4 // Args: +stack (auto-clean) / No prototype (3 stack arguments)
#define C_DrawFolder_                        x
#define C_DrawInfoWin_                       x
#define C_dude_run_                          0x17A0C // Args: non-stack / No prototype
#define C_dude_stand_                        0x17C60 // Args: non-stack / No prototype
#define C_can_see_                           0x12974 // Args: non-stack / _BOOL4 __usercall can_see_@<eax>(pobj *source@<eax>, pobj *target@<edx>)
#define C_editor_design_                     x
#define C_elapsed_time_                      x
#define C_EndLoad_                           x
#define C_endgame_movie_                     0x38C6C // Args: non-stack / No prototype
#define C_endgame_slideshow_                 0x38890 // Args: non-stack / No prototype
#define C_exec_script_proc_                  0x92904 // Args: non-stack / No prototype
#define C_floor_draw_                        0x9FB84 // Args: non-stack / No prototype
#define C_game_check_disk_space_             0x3D604 // Args: non-stack / No prototype
#define C_game_get_global_var_               0x3C750 // Args: non-stack / int __usercall game_get_global_var_@<eax>(int GVAR@<eax>)
#define C_game_set_global_var_               0x3C780 // Args: non-stack / int __usercall game_set_global_var_@<eax>(int GVAR@<eax>, int Value@<edx>)
#define C_gdialog_display_msg_               0x3E694 // Args: non-stack / No prototype
#define C_get_input_                         0xB35E8 // Args: non-stack / No prototype
#define C_get_time_                          x
#define C_getmsg_                            0x7711C // Args: non-stack / char *__usercall getmsg_@<eax>(int *msgfile@<eax>, int number@<ebx>, sMessage *msgdata@<edx>)
#define C_GetSlotList_                       x
#define C_gmouse_is_scrolling_               0x432AC // Args: non-stack / No prototype
#define C_gmovie_play_                       x
#define C_GNW_find_                          0xC3868 // Args: non-stack / No prototype
#define C_gsnd_build_weapon_sfx_name_        x
#define C_gsound_get_sound_ready_for_effect_ x
#define C_gsound_play_sfx_file_              0x49524 // Args: non-stack / No prototype
#define C_gsound_red_butt_press_             0x494EC // Args: non-stack / No prototype
#define C_InitLoadSave_                      x
#define C_intface_redraw_                    0x547B8 // Args: non-stack / No prototype
#define C_intface_toggle_item_state_         0x54EB8 // Args: non-stack / No prototype
#define C_intface_update_hit_points_         0x54838 // Args: non-stack / No prototype
#define C_intface_update_items_              0x54BA8 // Args: non-stack / int __usercall intface_update_items_@<eax>(int flag@<eax>)
#define C_intface_update_move_points_        0x54A64 // Args: non-stack / No prototype
#define C_intface_use_item_                  0x54FB0 // Args: non-stack / No prototype
#define C_inven_display_msg_                 0x66204 // Args: non-stack / No prototype
#define C_inven_left_hand_                   0x653B8 // Args: non-stack / No prototype
#define C_inven_right_hand_                  0x65378 // Args: non-stack / int __usercall inven_right_hand_@<eax>(pobj *source@<eax>)
#define C_inven_worn_                        0x653F8 // Args: non-stack / No prototype
#define C_isPartyMember_                     0x85A5C // Args: non-stack / No prototype
#define C_is_pc_flag_                        0x28C84 // Args: non-stack / No prototype
#define C_item_add_force_                    0x6A074 // Args: non-stack / int __usercall item_add_force_@<eax>(pobj *source@<eax>, iobj *item@<edx>, int count@<ebx>)
#define C_item_add_mult_                     x
#define C_item_ar_perk_                      0x6B990 // Args: non-stack / No prototype
#define C_item_c_curr_size_                  0x6C050 // Args: non-stack / No prototype
#define C_item_c_max_size_                   0x6C030 // Args: non-stack / No prototype
#define C_item_d_check_addict_               0xA0A4C // Args: non-stack / No prototype
#define C_item_d_take_drug_                  0x6C32C // Args: non-stack / signed int __usercall item_d_take_drug_@<eax>(pobj *source@<eax>, iobj *item@<edx>)
#define C_item_get_type_                     0x6A6E8 // Args: non-stack / item_type_ __usercall item_get_type_@<eax>(int item@<eax>)
#define C_item_m_cell_pid_                   0x6BA8C // Args: non-stack / No prototype
#define C_item_m_turn_off_                   0x6BED0 // Args: non-stack / No prototype
#define C_item_move_all_                     0x6A47C // Args: non-stack / int __usercall item_move_all_@<eax>(pobj *source@<eax>, pobj *target@<edx>)
#define C_item_move_force_                   0x6A474 // Args: non-stack / int __usercall item_move_force_@<eax>(pobj *source@<eax>, iobj *item@<ebx>, int count@<ecx>, pobj *target@<edx>)
#define C_item_mp_cost_                      0x6AB48 // Args: non-stack / No prototype
#define C_item_remove_mult_                  0x6A248 // Args: non-stack / int __usercall item_remove_mult_@<eax>(pobj *source@<eax>, iobj *item@<edx>, int count@<ebx>)
#define C_item_size_                         0x6A720 // Args: non-stack / No prototype
#define C_item_total_weight_                 0x6A9B8 // Args: non-stack / int __usercall item_total_weight_@<eax>(pobj *source@<eax>)
#define C_item_w_anim_code_                  0x6B62C // Args: non-stack / No prototype
#define C_item_w_anim_weap_                  0x6B06C // Args: non-stack / No prototype
#define C_item_w_is_2handed_                 x
#define C_item_w_perk_                       0x6B5DC // Args: non-stack / No prototype
#define C_item_w_try_reload_                 0x6B1BC // Args: non-stack / No prototype
#define C_item_weight_                       0x6A73C // Args: non-stack / int __usercall item_weight_@<eax>(iobj *item@<eax>)
#define C_is_within_perception_              0x264C0 // Args: non-stack / _BOOL4 __usercall is_within_perception_@<eax>(pobj *source@<eax>, pobj *target@<edx>)
#define C_is_pc_sneak_working_               0x28D0C // Args: non-stack / _BOOL4 __usercall is_pc_sneak_working_@<eax>()
#define C_light_get_tile_                    0x6CD78 // Args: non-stack / No prototype
#define C_ListDrvdStats_                     x
#define C_ListSkills_                        0x303A4 // Args: non-stack / No prototype
#define C_ListTraits_                        x
#define C_load_frame_                        x
#define C_LoadSlot_                          0x6FF7C // Args: non-stack / No prototype
#define C_main_game_loop_                    0x72D04 // Args: non-stack / No prototype
#define C_main_menu_loop_                    0x736CC // Args: non-stack / No prototype
#define C_make_path_func_                    0x15998 // Args: +stack (auto-clean) / int __userpurge make_path_func_@<eax>(pobj *source@<eax>, int target_tile@<ebx>, void *sad_rotation_ptr@<ecx>, int source_tile@<edx>, int a5, void *func)
#define C_make_straight_path_func_           0x15DD8 // Args: +stack (auto-clean) / No prototype
#define C_mem_free_                          0xAED88 // Args: non-stack / No prototype
#define C_mem_malloc_                        x
#define C_mem_realloc_                       0xAECC0 // Args: non-stack / No prototype
#define C_memmove_                           x
//#define C_memset_                          x - already in CStdFuncs.h
#define C_message_exit_                      x
#define C_message_load_                      x
#define C_message_search_                    x
#define C_mouse_click_in_                    0xB4930 // Args: non-stack / int __usercall mouse_click_in_@<eax>(int x_start@<eax>, int y_start@<edx>, int x_end@<ebx>, int y_end@<ecx>)
#define C_move_inventory_                    x
#define C_obj_blocking_at_                   0x7D2A0 // Args: non-stack / No prototype
#define C_obj_change_fid_                    x
#define C_obj_connect_                       0x7BB40 // Args: non-stack / No prototype
#define C_obj_destroy_                       0x8ACE8 // Args: non-stack / No prototype
#define C_obj_dist_                          0x7D418 // Args: non-stack / No prototype
#define C_obj_find_first_at_                 0x7CF9C // Args: non-stack / int __usercall obj_find_first_at_@<eax>(int elev@<eax>)
#define C_obj_find_next_at_                  0x7D020 // Args: non-stack / No prototype
#define C_obj_intersects_with_               0x7D9E0 // Args: non-stack / No prototype
#define C_obj_is_a_portal_                   x
#define C_obj_outline_object_                0x7D954 // Args: non-stack / No prototype
#define C_obj_process_seen_                  0x7DE34 // Args: non-stack / No prototype
#define C_obj_remove_outline_                0x7D990 // Args: non-stack / No prototype
#define C_obj_render_object_                 0x80818 // Args: non-stack / No prototype
#define C_obj_save_dude_                     x
#define C_obj_sight_blocking_at_             0x7D388 // Args: non-stack / No prototype
#define C_obj_top_environment_               0x7CE14 // Args: non-stack / pobj *__usercall obj_top_environment_@<eax>(pobj *source@<eax>)
#define C_obj_turn_off_outline_              0x7CA98 // Args: non-stack / No prototype
#define C_obj_turn_on_outline_               0x7CA7C // Args: non-stack / No prototype
#define C_obj_use_book_                      0x8AD38 // Args: non-stack / No prototype
#define C_opendir_                           x
#define C_pc_flag_off_                       0x28B7C // Args: non-stack / No prototype
#define C_pc_flag_on_                        0x28BC8 // Args: non-stack / No prototype
#define C_pc_flag_toggle_                    0x28C3C // Args: non-stack / No prototype
#define C_perk_level_                        0x86804 // Args: non-stack / int __usercall perk_level_@<eax>(int perk@<eax>)
#define C_perks_dialog_                      0x36398 // Args: non-stack / No prototype
#define C_PipStatus_                         0x875AC // Args: non-stack / No prototype
#define C_popLongStack_                      0x5BA5C // Args: non-stack / No prototype
#define C_PrintBasicStat_                    x
#define C_PrintLevelWin_                     x
#define C_process_bk_                        0xB3638 // Args: non-stack / No prototype
#define C_proto_dude_update_gender_          x
#define C_proto_ptr_                         0x9045C // Args: non-stack / int __usercall proto_ptr_@<eax>(int pid@<eax>, int buf@<edx>)
#define C_pushLongStack_                     0x5BA04 // Args: non-stack / No prototype
#define C_queue_add_                         0x90850 // Args: non-stack / signed int __usercall queue_add_@<eax>(int time@<eax>, int object@<edx>, int *extramem@<ebx>, int type@<ecx>)
#define C_queue_clear_type_                  0x90A54 // Args: non-stack / No prototype
#define C_queue_find_                        0x9096C // Args: non-stack / No prototype
#define C_queue_remove_this_                 0x90910 // Args: non-stack / No prototype
#define C_register_begin_                    0x13534 // Args: non-stack / No prototype
#define C_register_clear_                    0x136B8 // Args: non-stack / No prototype
#define C_register_end_                      0x13738 // Args: non-stack / No prototype
#define C_register_object_animate_           0x14330 // Args: non-stack / No prototype
#define C_register_object_change_fid_        0x14BA4 // Args: non-stack / No prototype
#define C_RestorePlayer_                     0x34AB8 // Args: non-stack / No prototype
#define C_roll_random_                       0x91480 // Args: non-stack / int __usercall roll_random_@<eax>(int min@<eax>, int max@<edx>)
#define C_SaveGame_                          x
#define C_SavePlayer_                        0x349E0 // Args: non-stack / No prototype
#define C_scr_exec_map_update_scripts_       0x94A50 // Args: non-stack / No prototype
#define C_scr_write_ScriptNode_              0x93880 // Args: non-stack / No prototype
#define C_set_game_time_                     0x9186C // Args: non-stack / void __usercall set_game_time_(int time_in_ticks@<eax>)
#define C_SexWindow_                         x
#define C_skill_dec_point_                   0x98608 // Args: non-stack / int __usercall skill_dec_point_@<eax>(pobj *source@<eax>, int skill@<edx>)
#define C_skill_get_tags_                    0x983E4 // Args: non-stack / No prototype
#define C_skill_inc_point_                   0x98564 // Args: non-stack / int __usercall skill_inc_point_@<eax>(pobj *source@<eax>, int skill@<edx>)
#define C_skill_level_                       0x98408 // Args: non-stack / int __usercall skill_level_@<eax>(pobj *source@<eax>, int skill@<edx>)
#define C_skill_set_tags_                    0x983C0 // Args: non-stack / No prototype
#define C_soundUpdate_                       0x9C188 // Args: non-stack / No prototype
//#define C_sprintf_                         x - already in CStdFuncs.h
#define C_srcCopy_                           x
#define C_scr_set_objs_                      0x91DE0 // Args: non-stack / signed int __usercall scr_set_objs_@<eax>(int sid@<eax>, pobj *target@<ebx>, pobj *source@<edx>)
#define C_stat_get_bonus_                    0x9C66C // Args: non-stack / No prototype
#define C_stat_level_                        0x9C4E8 // Args: non-stack / int __usercall stat_level_@<eax>(pobj *source@<eax>, int stat@<edx>)
#define C_stat_pc_add_experience_            0x9CC5C // Args: non-stack / No prototype
#define C_stat_pc_get_                       0x9CAF4 // Args: non-stack / No prototype
#define C_stat_pc_min_exp_                   0x9CB7C // Args: non-stack / No prototype
//#define C_strcmp_                          x - already in CStdFuncs.h
//#define C_strncpy_                         x - already in CStdFuncs.h
#define C_switch_hand_                       x
#define C_talk_to_pressed_barter_            0x41034 // Args: non-stack / No prototype
#define C_text_font_                         0xC191C // Args: non-stack / int __usercall text_font_@<eax>(int fontnum@<eax>)
#define C_tile_disable_scroll_blocking_      0x9E8A0 // Args: non-stack / No prototype
#define C_tile_refresh_display_              0x9DEDC // Args: non-stack / No prototype
#define C_tile_refresh_rect_                 0x9DEC4 // Args: non-stack / No prototype
#define C_tile_scroll_to_                    0xA03E4 // Args: non-stack / No prototype
#define C_trait_get_                         0xA0614 // Args: non-stack / No prototype
#define C_trait_set_                         0xA0608 // Args: non-stack / No prototype
#define C_transSrcCopy_                      x
#define C__word_wrap_                        x
#define C_win_delete_                        x
#define C_win_draw_                          0xC2F40 // Args: non-stack / int __usercall win_draw_@<eax>(int WinRef@<eax>)
#define C_win_draw_rect_                     0xC2F64 // Args: non-stack / int __usercall win_draw_rect_@<eax>(int WinRef@<eax>, int *WinRect@<edx>)
#define C_win_get_buf_                       0xC3890 // Args: non-stack / int __usercall win_get_buf_@<eax>(int WinRef@<eax>)
#define C_win_get_num_i_                     x
#define C_win_line_                          x
#define C_win_print_                         0xC2830 // Args: +stack (auto-clean) / No prototype (4 stack arguments)
#define C_win_register_button_               0xC4180 // Args: +stack (auto-clean) / signed int __userpurge win_register_button_@<eax>(DWORD GNWID@<eax>, int Ypos@<ebx>, int Width@<ecx>, int Xpos@<edx>, int Height, int HovOn, int HovOff, int ButtDown, int ButtUp, int PicUp, int PicDown, int Unk, int flags)
#define C_win_register_text_button_          x
#define C_win_width_                         x

// //////////////////////////////////////////////////////////////////////////////
// DATA SECTION HERE

#define D__ambient_light           0x1059EC
#define D__anim_set                0x15EEFC
#define D__art_name                x
#define D__art_skilldex            x
#define D__art_vault_guy_num       x
#define D__art_vault_person_nums   x
#define D__barter_back_win         0x19B834
#define D__bbox                    0x1057F4
#define D__bckgnd                  x
#define D__BlueColor               0x2A1A2F
#define D__btable                  0x19B828
#define D__colorTable              x
#define D__combat_free_move        0x16A82C
#define D__combat_highlight        0x16A828
#define D__combat_list             0x16A810
#define D__combat_state            0xFEE78
#define D__combat_turn_running     0xFEE74
#define D__crit_succ_eff           0xFEEA8
#define D__critter_db_handle       x
#define D__crnt_func               0x2612A4
#define D__curr_crit_num           0x16AA4C
#define D__curr_crit_list          0x16AA50
#define D__curr_font_num           0x13A374
#define D__curr_pc_stat            0x263CA0
#define D__curr_rot                x
#define D__curr_stack              0x19B818
#define D__current_database        x
#define D__DARK_GREEN_Color        x
#define D__DARK_GREY_Color         x
#define D__dialogue_state          0x1051F4
#define D__dialogue_switch_mode    0x1051F8
#define D__drug_pid                0x1059CC
#define D__edit_win                x
#define D__Educated                0x16D1B8
#define D__Experience_             0x263CA8
#define D__fallout_game_time       0x1079DC
#define D__flptr                   x
#define D__frame_time              x
#define D__free_perk               0x16D8BD
#define D__frstc_draw1             x
#define D__game_global_vars        0x1051BC
#define D__game_user_wants_to_quit 0x1051C8
#define D__gdNumOptions            0x10538C
#define D__GInfo                   x
#define D__gIsSteal                0x107EE0
#define D__glblmode                x
#define D__gmouse_current_cursor   x
#define D__GreenColor              0x2A1DF0
#define D__grid_length             0x2678F0
#define D__grid_size               0x2678F4
#define D__grid_width              0x2678E4
#define D__grphbmp                 x
#define D__hash_is_on              x
#define D__holo_flag               0x261335
#define D__hot_line_count          0x261290
#define D__i_lhand                 0x19B840
#define D__i_rhand                 0x19B84C
#define D__i_wid                   0x19B848
#define D__i_worn                  0x19B83C
#define D__info_line               x
#define D__intfaceEnabled          0x10571C
#define D__interfaceWindow         0x105818
#define D__inven_dude              0x10584C
#define D__inven_pid               0x105850
#define D__itemButtonItems         0x193FD0
#define D__itemCurrentItem         0x105784
#define D__kb_lock_flags           x
#define D__last_level              0x16D830
#define D__Level_                  0x263CA4
#define D__Lifegiver               0x16D1E0
#define D__LIGHT_GREY_Color        x
#define D__list_com                0x16A814
#define D__list_total              0x16A80C
#define D__LSData                  x
#define D__lsgmesg                 x
#define D__lsgwin                  x
#define D__main_window             0x105C1C
#define D__map_elevation           0x105C90
#define D__max_ptr                 0x16CE70
#define D__mesg                    x
#define D__msg_path                x
#define D__Mutate_                 0x16D240
#define D__name_color              0x16AA74
#define D__name_font               0x16AA7C
#define D__obj_dude                0x25DCA8
#define D__obj_egg                 0x25DC90
#define D__old_fid1                x
#define D__old_str1                x
#define D__objectTable             0x236980
#define D__outlined_object         0x1055E4
#define D__patches                 x
#define D__pc_name                 0x16AA8C
#define D__pc_proto                0x10764C
#define D__PeanutButter            x
#define D__perk_data               0x1064B4
#define D__perk_lev                0x260FF4
#define D__proto_main_msg_file     0x261428
#define D__ptable                  0x19B814
#define D__pud                     0x19B780
#define D__queue                   0x2615DC
#define D__quick_done              x
#define D__rad_bonus               0x104F70
#define D__refresh_enabled         0x1084CC
#define D__RedColor                0x2A9610
#define D__sfx_file_name           0x193F11
#define D__slot_cursor             x
#define D__sneak_working           0x16AAAC
#define D__square_length           0x2678F8
#define D__square_offx             0x2678C4
#define D__square_offy             0x2678C8
#define D__square_size             0x2678E0
#define D__square_width            0x267900
#define D__square_x                0x2678C0
#define D__square_y                0x2678BC
#define D__stack                   0x19B71C
#define D__stack_offset            0x19B6C4
#define D__sthreads                0x1073B8
#define D__str                     0x210D60
#define D__Tag_                    0x16D23C
#define D__tag_skill               0x263680
#define D__target_curr_stack       0x19B82C
#define D__target_pud              0x19B824
#define D__target_stack            0x19B744
#define D__target_stack_offset     0x19B6EC
#define D__text_char_width         0x13A388 // Args: non-stack / No prototype
#define D__text_height             0x13A380 // Args: non-stack (Should be on stack (__cdecl) but I don't see PUSHes, so I guess I'll suppose __cdecl is a mistake? _text_width is a __fastcall, so maybe this was a mistake, hopefully) / int (__cdecl *text_height)(_DWORD)
#define D__text_to_buf             0x13A37C // Args: +stack (?-clean) / void __userpurge text_to_buf(int ToSurface@<eax>, int TxtWidth@<ebx>, int ToWidth@<ecx>, char *DisplayText@<edx>, int ColorIndex)
#define D__text_width              0x13A384 // Args: non-stack / int (__fastcall *text_width)(_DWORD, _DWORD)
#define D__ticker                  x
#define D__tile_intensity          0x19B86C
#define D__title_color             0x16AA80
#define D__title_font              0x16AA78

#endif //F1DPPATCHER_FALLOUTENGINE_H
