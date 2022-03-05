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
// his modification of the original sFall1 by Timeslip.

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
//   If there is no prototype, write "No prototype" after the forward slash.
//
// Why have the prototype if we don't want to know how to use the function? Because it says the calling convention,
// which is useful to know sometimes (like with a function which I think has the prototype calling convention written
// wrong, but the number of parameters are there - sum up: was useful, so now I'm doing that always).

// //////////////////////////////////////////////////////////////////////////////
// CODE SECTION HERE

#define C_action_get_an_object_              x
#define C_action_loot_container_             x
#define C_action_use_an_item_on_object_      x
#define C_AddHotLines_                       x
#define C_adjust_ac_                         x
#define C_adjust_fid_                        x
#define C_art_alias_num_                     x
#define C_art_exists_                        x
#define C_art_flush_                         x
#define C_art_frame_data_                    x
#define C_art_frame_length_                  x
#define C_art_frame_width_                   x
#define C_art_id_                            0x193FC // Args: +stack / int __userpurge art_id_@<eax>(int ObjType@<eax>, int Index@<edx>, int ID1@<ecx>, int ID2@<ebx>, int ID3)
#define C_art_ptr_lock_                      x
#define C_art_ptr_lock_data_                 0x18968 // Args: non-stack / No prototype
#define C_art_ptr_unlock_                    0x18A5C // Args: non-stack / int __usercall art_ptr_unlock_@<eax>(int art_ptr@<eax>)
#define C_buf_fill_                          x
#define C_buf_to_buf_                        0xBD6F4 // Args: +stack / int __cdecl buf_to_buf_(int from, int width, int height, int from_width, int to, int to_width)
#define C_closedir_                          x
#define C_combat_ai_                         x
#define C_combat_is_shot_blocked_            x
#define C_combat_should_end_                 x
#define C_combat_turn_                       x
#define C_config_set_value_                  x
#define C_container_exit_                    0x69250 // Args: non-stack / No prototype
#define C_credits_                           x
#define C_credits_get_next_line_             x
#define C_critter_body_type_                 0x28778 // Args: non-stack / int __usercall critter_body_type_@<eax>(pobj *source@<eax>)
#define C_critter_is_dead_                   0x286CC // Args: non-stack / No prototype
#define C_critter_kill_                      x
#define C_critter_name_                      x
#define C_critter_pc_set_name_               x
#define C_db_close_                          x
#define C_db_create_database_                x
#define C_db_current_                        x
#define C_db_dir_entry_                      x
#define C_db_fclose_                         x
#define C_db_freadIntCount_                  x
#define C_db_fwriteIntCount_                 x
#define C_db_init_database_                  x
#define C_db_init_hash_table_                x
#define C_db_init_patches_                   x
#define C_db_select_                         x
#define C_debug_register_env_                x
#define C_dialog_out_                        0x1BFA0 // Args: +stack / No prototype (5 stack arguments)
#define C_display_inventory_                 0x639E8 // Args: non-stack / int __usercall display_inventory_@<eax>(int inventory_offset@<eax>, int mode@<ebx>, int visible_offset@<edx>)
#define C_display_print_                     0x2C05C // Args: non-stack / No prototype
#define C_display_scroll_down_               x
#define C_display_scroll_up_                 x
#define C_display_stats_                     0x654F4 // Args: non-stack / No prototype
#define C_display_table_inventories_         0x683C8 // Args: non-stack / int __usercall display_table_inventories_@<eax>(int GNWID@<eax>, pobj *target@<ebx>, int a3@<ecx>, pobj *source@<edx>)
#define C_display_target_inventory_          0x63E90 // Args: non-stack / void __usercall display_target_inventory_(int inventory_offset@<eax>, int visible_offset@<edx>, Inventory *target_inventory@<ebx>, int mode@<ecx>)
#define C_DOSCmdLineDestroy_                 x
#define C_draw_box_                          x
#define C_DrawFolder_                        x
#define C_DrawInfoWin_                       x
#define C_dude_stand_                        x
#define C_editor_design_                     x
#define C_elapsed_time_                      x
#define C_EndLoad_                           x
#define C_endgame_movie_                     x
#define C_endgame_slideshow_                 x
#define C_exec_script_proc_                  x
#define C_game_get_global_var_               0x3C750 // Args: non-stack / int __usercall game_get_global_var_@<eax>(int GVAR@<eax>)
#define C_game_set_global_var_               0x3C780 // Args: non-stack / int __usercall game_set_global_var_@<eax>(int GVAR@<eax>, int Value@<edx>)
#define C_gdialog_display_msg_               0x3E694 // Args: non-stack / No prototype
#define C_get_input_                         x
#define C_get_time_                          x
#define C_getmsg_                            x
#define C_GetSlotList_                       x
#define C_gmouse_is_scrolling_               0x432AC // Args: non-stack / No prototype
#define C_gmovie_play_                       x
#define C_GNW_find_                          0xC3868 // Args: non-stack / No prototype
#define C_gsnd_build_weapon_sfx_name_        x
#define C_gsound_get_sound_ready_for_effect_ x
#define C_gsound_play_sfx_file_              0x49524 // Args: non-stack / No prototype
#define C_gsound_red_butt_press_             0x494EC // Args: non-stack / No prototype
#define C_InitLoadSave_                      x
#define C_intface_redraw_                    x
#define C_intface_toggle_item_state_         0x54EB8 // Args: non-stack / No prototype
#define C_intface_update_hit_points_         0x54838 // Args: non-stack / No prototype
#define C_intface_update_items_              0x54BA8 // Args: non-stack / int __usercall intface_update_items_@<eax>(int flag@<eax>)
#define C_intface_update_move_points_        0x54A64 // Args: non-stack / No prototype
#define C_intface_use_item_                  0x54FB0 // Args: non-stack / No prototype
#define C_inven_display_msg_                 x
#define C_inven_left_hand_                   x
#define C_inven_right_hand_                  x
#define C_inven_worn_                        x
#define C_isPartyMember_                     x
#define C_item_add_force_                    0x6A074 // Args: non-stack / int __usercall item_add_force_@<eax>(pobj *source@<eax>, iobj *item@<edx>, int count@<ebx>)
#define C_item_add_mult_                     x
#define C_item_c_curr_size_                  0x6C050 // Args: non-stack / No prototype
#define C_item_c_max_size_                   0x6C030 // Args: non-stack / No prototype
#define C_item_d_check_addict_               x
#define C_item_d_take_drug_                  0x6C32C // Args: non-stack / signed int __usercall item_d_take_drug_@<eax>(pobj *source@<eax>, iobj *item@<edx>)
#define C_item_get_type_                     0x6A6E8 // Args: non-stack / item_type_ __usercall item_get_type_@<eax>(int item@<eax>)
#define C_item_m_cell_pid_                   0x6BA8C // Args: non-stack / No prototype
#define C_item_m_turn_off_                   x
#define C_item_move_all_                     0x6A47C // Args: non-stack / int __usercall item_move_all_@<eax>(pobj *source@<eax>, pobj *target@<edx>)
#define C_item_move_force_                   0x6A474 // Args: non-stack / int __usercall item_move_force_@<eax>(pobj *source@<eax>, iobj *item@<ebx>, int count@<ecx>, pobj *target@<edx>)
#define C_item_mp_cost_                      0x6AB48 // Args: non-stack / No prototype
#define C_item_remove_mult_                  0x6A248 // Args: non-stack / int __usercall item_remove_mult_@<eax>(pobj *source@<eax>, iobj *item@<edx>, int count@<ebx>)
#define C_item_size_                         0x6A720 // Args: non-stack / No prototype
#define C_item_total_weight_                 0x6A9B8 // Args: non-stack / int __usercall item_total_weight_@<eax>(pobj *source@<eax>)
#define C_item_w_anim_code_                  x
#define C_item_w_anim_weap_                  x
#define C_item_w_is_2handed_                 x
#define C_item_w_try_reload_                 0x6B1BC // Args: non-stack / No prototype
#define C_item_weight_                       0x6A73C // Args: non-stack / int __usercall item_weight_@<eax>(iobj *item@<eax>)
#define C_ListDrvdStats_                     x
#define C_ListSkills_                        x
#define C_ListTraits_                        x
#define C_load_frame_                        x
#define C_LoadSlot_                          x
#define C_main_game_loop_                    x
#define C_main_menu_loop_                    x
#define C_mem_free_                          x
#define C_mem_malloc_                        x
#define C_memmove_                           x
//#define C_memset_                          x - already in CStdFuncs.h
#define C_message_exit_                      x
#define C_message_load_                      x
#define C_message_search_                    x
#define C_mouse_click_in_                    0xB4930 // Args: non-stack / int __usercall mouse_click_in_@<eax>(int x_start@<eax>, int y_start@<edx>, int x_end@<ebx>, int y_end@<ecx>)
#define C_move_inventory_                    x
#define C_obj_change_fid_                    x
#define C_obj_connect_                       0x7BB40 // Args: non-stack / No prototype
#define C_obj_destroy_                       0x8ACE8 // Args: non-stack / No prototype
#define C_obj_dist_                          x
#define C_obj_find_first_at_                 x
#define C_obj_find_next_at_                  x
#define C_obj_is_a_portal_                   x
#define C_obj_outline_object_                x
#define C_obj_remove_outline_                x
#define C_obj_save_dude_                     x
#define C_obj_top_environment_               0x7CE14 // Args: non-stack / pobj *__usercall obj_top_environment_@<eax>(pobj *source@<eax>)
#define C_obj_turn_off_outline_              x
#define C_obj_turn_on_outline_               x
#define C_obj_use_book_                      x
#define C_opendir_                           x
#define C_pc_flag_off_                       x
#define C_pc_flag_on_                        x
#define C_pc_flag_toggle_                    x
#define C_perk_level_                        0x86804 // Args: non-stack / int __usercall perk_level_@<eax>(int perk@<eax>)
#define C_perks_dialog_                      x
#define C_PipStatus_                         x
#define C_PrintBasicStat_                    x
#define C_PrintLevelWin_                     x
#define C_process_bk_                        x
#define C_proto_dude_update_gender_          x
#define C_proto_ptr_                         0x9045C // Args: non-stack / int __usercall proto_ptr_@<eax>(int pid@<eax>, int buf@<edx>)
#define C_queue_add_                         x
#define C_queue_clear_type_                  x
#define C_queue_find_                        x
#define C_queue_remove_this_                 x
#define C_register_begin_                    0x13534 // Args: non-stack / No prototype
#define C_register_clear_                    0x136B8 // Args: non-stack / No prototype
#define C_register_end_                      0x13738 // Args: non-stack / No prototype
#define C_register_object_animate_           0x14330 // Args: non-stack / No prototype
#define C_register_object_change_fid_        x
#define C_RestorePlayer_                     x
#define C_SaveGame_                          x
#define C_SavePlayer_                        x
#define C_scr_exec_map_update_scripts_       0x94A50 // Args: non-stack / No prototype
#define C_scr_write_ScriptNode_              x
#define C_set_game_time_                     x
#define C_SexWindow_                         x
#define C_skill_dec_point_                   x
#define C_skill_get_tags_                    x
#define C_skill_inc_point_                   x
#define C_skill_level_                       x
#define C_skill_set_tags_                    x
//#define C_sprintf_                         x - already in CStdFuncs.h
#define C_srcCopy_                           x
#define C_stat_get_bonus_                    x
#define C_stat_level_                        0x9C4E8 // Args: non-stack / int __usercall stat_level_@<eax>(pobj *source@<eax>, int stat@<edx>)
#define C_stat_pc_add_experience_            x
#define C_stat_pc_get_                       x
#define C_stat_pc_min_exp_                   x
//#define C_strcmp_                          x - already in CStdFuncs.h
//#define C_strncpy_                         x - already in CStdFuncs.h
#define C_switch_hand_                       x
#define C_text_font_                         0xC191C // Args: non-stack / int __usercall text_font_@<eax>(int fontnum@<eax>)
#define C_tile_refresh_display_              x
#define C_tile_refresh_rect_                 x
#define C_tile_scroll_to_                    x
#define C_trait_get_                         x
#define C_trait_set_                         x
#define C_transSrcCopy_                      x
#define C__word_wrap_                        x
#define C_win_delete_                        x
#define C_win_draw_                          0xC2F40 // Args: non-stack / int __usercall win_draw_@<eax>(int WinRef@<eax>)
#define C_win_draw_rect_                     0xC2F64 // Args: non-stack / int __usercall win_draw_rect_@<eax>(int WinRef@<eax>, int *WinRect@<edx>)
#define C_win_get_buf_                       0xC3890 // Args: non-stack / int __usercall win_get_buf_@<eax>(int WinRef@<eax>)
#define C_win_get_num_i_                     x
#define C_win_line_                          x
#define C_win_print_                         x
#define C_win_register_button_               0xC4180 // Args: +stack / signed int __userpurge win_register_button_@<eax>(DWORD GNWID@<eax>, int Ypos@<ebx>, int Width@<ecx>, int Xpos@<edx>, int Height, int HovOn, int HovOff, int ButtDown, int ButtUp, int PicUp, int PicDown, int Unk, int flags)
#define C_win_register_text_button_          x
#define C_win_width_                         x

// //////////////////////////////////////////////////////////////////////////////
// DATA SECTION HERE

#define D__art_name                x
#define D__art_skilldex            x
#define D__art_vault_guy_num       x
#define D__art_vault_person_nums   x
#define D__barter_back_win         0x19B834
#define D__bckgnd                  x
#define D__btable                  0x19B828
#define D__colorTable              x
#define D__combat_free_move        x
#define D__combat_highlight        x
#define D__combat_list             x
#define D__combat_state            0xFEE78
#define D__combat_turn_running     x
#define D__crit_succ_eff           0xFEEA8
#define D__critter_db_handle       x
#define D__crnt_func               x
#define D__curr_font_num           0x13A374
#define D__curr_pc_stat            x
#define D__curr_rot                x
#define D__curr_stack              0x19B818
#define D__current_database        x
#define D__DARK_GREEN_Color        x
#define D__DARK_GREY_Color         x
#define D__drug_pid                x
#define D__edit_win                x
#define D__Educated                x
#define D__Experience_             x
#define D__fallout_game_time       x
#define D__flptr                   x
#define D__frame_time              x
#define D__free_perk               x
#define D__frstc_draw1             x
#define D__game_global_vars        x
#define D__game_user_wants_to_quit 0x1051C8
#define D__gdNumOptions            x
#define D__GInfo                   x
#define D__gIsSteal                0x107EE0
#define D__glblmode                x
#define D__gmouse_current_cursor   x
#define D__GreenColor              0x2A1DF0
#define D__grphbmp                 x
#define D__hash_is_on              x
#define D__holo_flag               x
#define D__hot_line_count          x
#define D__i_lhand                 0x19B840
#define D__i_rhand                 0x19B84C
#define D__i_wid                   0x19B848
#define D__i_worn                  0x19B83C
#define D__info_line               x
#define D__intfaceEnabled          0x10571C
#define D__interfaceWindow         0x105818
#define D__inven_dude              0x10584C
#define D__inven_pid               x
#define D__itemButtonItems         0x193FD0
#define D__itemCurrentItem         0x105784
#define D__kb_lock_flags           x
#define D__last_level              x
#define D__Level_                  x
#define D__Lifegiver               x
#define D__LIGHT_GREY_Color        x
#define D__list_com                x
#define D__list_total              x
#define D__LSData                  x
#define D__lsgmesg                 x
#define D__lsgwin                  x
#define D__main_window             x
#define D__map_elevation           x
#define D__max_ptr                 0x16CE70
#define D__mesg                    x
#define D__msg_path                x
#define D__Mutate_                 x
#define D__name_color              x
#define D__name_font               x
#define D__obj_dude                0x25DCA8
#define D__old_fid1                x
#define D__old_str1                x
#define D__outlined_object         x
#define D__patches                 x
#define D__pc_name                 x
#define D__pc_proto                x
#define D__PeanutButter            x
#define D__perk_lev                x
#define D__proto_main_msg_file     x
#define D__ptable                  0x19B814
#define D__pud                     x
#define D__queue                   x
#define D__quick_done              x
#define D__RedColor                0x2A9610
#define D__slot_cursor             x
#define D__sneak_working           x
#define D__stack                   0x19B71C
#define D__stack_offset            0x19B6C4
#define D__sthreads                x
#define D__str                     x
#define D__Tag_                    x
#define D__tag_skill               x
#define D__target_curr_stack       0x19B82C
#define D__target_pud              0x19B824
#define D__target_stack            0x19B744
#define D__target_stack_offset     0x19B6EC
#define D__text_char_width         0x13A388 // Args: non-stack / No prototype
#define D__text_height             0x13A380 // Args: non-stack (Should be on stack (__cdecl) but I don't see PUSHes, so I guess I'll suppose __cdecl is a mistake? _text_width is a __fastcall, so maybe this was a mistake, hopefully) / int (__cdecl *text_height)(_DWORD)
#define D__text_to_buf             0x13A37C // Args: +stack / void __userpurge text_to_buf(int ToSurface@<eax>, int TxtWidth@<ebx>, int ToWidth@<ecx>, char *DisplayText@<edx>, int ColorIndex)
#define D__text_width              0x13A384 // Args: non-stack / int (__fastcall *text_width)(_DWORD, _DWORD)
#define D__ticker                  x
#define D__title_color             x
#define D__title_font              x

#endif //F1DPPATCHER_FALLOUTENGINE_H
