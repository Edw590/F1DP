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

#ifndef F1DPPATCHER_FALLOUTENGINE_H
#define F1DPPATCHER_FALLOUTENGINE_H



#include <stdint.h>

// The Xs are so that if a macro that doesn't have an address on it yet is used, an error will be thrown instantly.
// When putting the address, just add the 0 before the X (which I just took out for the errors to appear) and that's it.

// F_-started constants are for function addresses, and D_-started constants are for data addresses. This is so
// that on code, it's easy to know to which segment the macro belongs - just look at the prefix.

// Also, ALL function macros MUST have a comment on them saying if the arguments are passed on registers, registers +
// on the stack, or only on the stack, or if it has no args at all, but as it's not easy to know that, if there are no
// pushes for the function, just say it's registers - this is not to know how to use the function, it's just to know if
// one can use pushes and pops near the function or not (only if the function doesn't have arguments on the stack).

#define F_action_get_an_object_              x
#define F_action_loot_container_             x
#define F_action_use_an_item_on_object_      x
#define F_AddHotLines_                       x
#define F_adjust_ac_                         x
#define F_adjust_fid_                        x
#define F_art_alias_num_                     x
#define F_art_exists_                        x
#define F_art_flush_                         x
#define F_art_frame_data_                    x
#define F_art_frame_length_                  x
#define F_art_frame_width_                   x
#define F_art_id_                            0x193FC // Args: regs + stack
#define F_art_ptr_lock_                      x
#define F_art_ptr_lock_data_                 0x18968 // Args: regs
#define F_art_ptr_unlock_                    x
#define F_buf_fill_                          x
#define F_buf_to_buf_                        x
#define F_closedir_                          x
#define F_combat_ai_                         x
#define F_combat_is_shot_blocked_            x
#define F_combat_should_end_                 x
#define F_combat_turn_                       x
#define F_config_set_value_                  x
#define F_container_exit_                    0x69250 // Args: regs
#define F_credits_                           x
#define F_credits_get_next_line_             x
#define F_critter_body_type_                 0x28778 // Args: regs
#define F_critter_is_dead_                   x
#define F_critter_kill_                      x
#define F_critter_name_                      x
#define F_critter_pc_set_name_               x
#define F_db_close_                          x
#define F_db_create_database_                x
#define F_db_current_                        x
#define F_db_dir_entry_                      x
#define F_db_fclose_                         x
#define F_db_freadIntCount_                  x
#define F_db_fwriteIntCount_                 x
#define F_db_init_database_                  x
#define F_db_init_hash_table_                x
#define F_db_init_patches_                   x
#define F_db_select_                         x
#define F_debug_register_env_                x
#define F_dialog_out_                        0x1BFA0 // Args: regs(?) + stack
#define F_display_inventory_                 0x639E8 // Args: regs
#define F_display_print_                     x
#define F_display_scroll_down_               x
#define F_display_scroll_up_                 x
#define F_display_stats_                     x
#define F_display_table_inventories_         x
#define F_display_target_inventory_          0x63E90 // Args: regs
#define F_DOSCmdLineDestroy_                 x
#define F_draw_box_                          x
#define F_DrawFolder_                        x
#define F_DrawInfoWin_                       x
#define F_dude_stand_                        x
#define F_editor_design_                     x
#define F_elapsed_time_                      x
#define F_EndLoad_                           x
#define F_endgame_movie_                     x
#define F_endgame_slideshow_                 x
#define F_exec_script_proc_                  x
#define F_game_get_global_var_               x
#define F_game_set_global_var_               x
#define F_gdialog_display_msg_               x
#define F_get_input_                         x
#define F_get_time_                          x
#define F_getmsg_                            x
#define F_GetSlotList_                       x
#define F_gmouse_is_scrolling_               x
#define F_gmovie_play_                       x
#define F_GNW_find_                          x
#define F_gsnd_build_weapon_sfx_name_        x
#define F_gsound_get_sound_ready_for_effect_ x
#define F_gsound_play_sfx_file_              x
#define F_gsound_red_butt_press_             0x494EC // Args: regs
#define F_InitLoadSave_                      x
#define F_intface_redraw_                    x
#define F_intface_toggle_item_state_         x
#define F_intface_update_hit_points_         x
#define F_intface_update_items_              x
#define F_intface_update_move_points_        x
#define F_intface_use_item_                  x
#define F_inven_display_msg_                 x
#define F_inven_left_hand_                   x
#define F_inven_right_hand_                  x
#define F_inven_worn_                        x
#define F_isPartyMember_                     x
#define F_item_add_force_                    x
#define F_item_add_mult_                     x
#define F_item_c_curr_size_                  0x6C050 // Args: regs
#define F_item_c_max_size_                   0x6C030 // Args: regs
#define F_item_d_check_addict_               x
#define F_item_d_take_drug_                  x
#define F_item_get_type_                     0x6A6E8 // Args: regs
#define F_item_m_cell_pid_                   x
#define F_item_m_turn_off_                   x
#define F_item_move_all_                     0x6A47C // Args: regs
#define F_item_move_force_                   x
#define F_item_mp_cost_                      x
#define F_item_remove_mult_                  x
#define F_item_size_                         x
#define F_item_total_weight_                 0x6A9B8 // Args: regs
#define F_item_w_anim_code_                  x
#define F_item_w_anim_weap_                  x
#define F_item_w_is_2handed_                 x
#define F_item_w_try_reload_                 x
#define F_item_weight_                       0x6A73C // Args: regs
#define F_ListDrvdStats_                     x
#define F_ListSkills_                        x
#define F_ListTraits_                        x
#define F_load_frame_                        x
#define F_LoadSlot_                          x
#define F_main_game_loop_                    x
#define F_main_menu_loop_                    x
#define F_mem_free_                          x
#define F_mem_malloc_                        x
#define F_memmove_                           x
//#define F_memset_                            x - already in CStdFuncs.h
#define F_message_exit_                      x
#define F_message_load_                      x
#define F_message_search_                    x
#define F_mouse_click_in_                    x
#define F_move_inventory_                    x
#define F_obj_change_fid_                    x
#define F_obj_connect_                       x
#define F_obj_destroy_                       x
#define F_obj_dist_                          x
#define F_obj_find_first_at_                 x
#define F_obj_find_next_at_                  x
#define F_obj_is_a_portal_                   x
#define F_obj_outline_object_                x
#define F_obj_remove_outline_                x
#define F_obj_save_dude_                     x
#define F_obj_top_environment_               0x7CE14 // Args: regs
#define F_obj_turn_off_outline_              x
#define F_obj_turn_on_outline_               x
#define F_obj_use_book_                      x
#define F_opendir_                           x
#define F_pc_flag_off_                       x
#define F_pc_flag_on_                        x
#define F_pc_flag_toggle_                    x
#define F_perk_level_                        x // Args: regs
#define F_perks_dialog_                      x
#define F_PipStatus_                         x
#define F_PrintBasicStat_                    x
#define F_PrintLevelWin_                     x
#define F_process_bk_                        x
#define F_proto_dude_update_gender_          x
#define F_proto_ptr_                         x
#define F_queue_add_                         x
#define F_queue_clear_type_                  x
#define F_queue_find_                        x
#define F_queue_remove_this_                 x
#define F_register_begin_                    x
#define F_register_clear_                    x
#define F_register_end_                      x
#define F_register_object_animate_           x
#define F_register_object_change_fid_        x
#define F_RestorePlayer_                     x
#define F_SaveGame_                          x
#define F_SavePlayer_                        x
#define F_scr_exec_map_update_scripts_       x
#define F_scr_write_ScriptNode_              x
#define F_set_game_time_                     x
#define F_SexWindow_                         x
#define F_skill_dec_point_                   x
#define F_skill_get_tags_                    x
#define F_skill_inc_point_                   x
#define F_skill_level_                       x
#define F_skill_set_tags_                    x
#define F_sprintf_                           x // Args: stack
#define F_srcCopy_                           x
#define F_stat_get_bonus_                    x
#define F_stat_level_                        0x9C4E8 // Args: regs
#define F_stat_pc_add_experience_            x
#define F_stat_pc_get_                       x
#define F_stat_pc_min_exp_                   x
//#define F_strcmp_                            x - already in CStdFuncs.h
//#define F_strncpy_                           x - already in CStdFuncs.h
#define F_switch_hand_                       x
#define F_text_font_                         x
#define F_tile_refresh_display_              x
#define F_tile_refresh_rect_                 x
#define F_tile_scroll_to_                    x
#define F_trait_get_                         x
#define F_trait_set_                         x
#define F_transSrcCopy_                      x
#define F__word_wrap_                        x
#define F_win_delete_                        x
#define F_win_draw_                          0xC2F40 // Args: regs
#define F_win_draw_rect_                     x
#define F_win_get_buf_                       x
#define F_win_get_num_i_                     x
#define F_win_line_                          x
#define F_win_print_                         x
#define F_win_register_button_               0xC4180 // Args: regs + stack
#define F_win_register_text_button_          x
#define F_win_width_                         x

// //////////////////////////////////////////////////////////////////////////////

#define D__art_name                x
#define D__art_skilldex            x
#define D__art_vault_guy_num       x
#define D__art_vault_person_nums   x
#define D__barter_back_win         x
#define D__bckgnd                  x
#define D__btable                  x
#define D__colorTable              x
#define D__combat_free_move        x
#define D__combat_highlight        x
#define D__combat_list             x
#define D__combat_state            x
#define D__combat_turn_running     x
#define D__crit_succ_eff           x
#define D__critter_db_handle       x
#define D__crnt_func               x
#define D__curr_font_num           x
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
#define D__game_user_wants_to_quit x
#define D__gdNumOptions            x
#define D__GInfo                   x
#define D__gIsSteal                0x107EE0
#define D__glblmode                x
#define D__gmouse_current_cursor   x
#define D__GreenColor              x
#define D__grphbmp                 x
#define D__hash_is_on              x
#define D__holo_flag               x
#define D__hot_line_count          x
#define D__i_lhand                 0x19B840
#define D__i_rhand                 0x19B84C
#define D__i_wid                   0x19B848
#define D__i_worn                  0x19B83C
#define D__info_line               x
#define D__intfaceEnabled          x
#define D__interfaceWindow         x
#define D__inven_dude              0x10584C
#define D__inven_pid               x
#define D__itemButtonItems         x
#define D__itemCurrentItem         x
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
#define D__max_ptr                 x
#define D__mesg                    x
#define D__msg_path                x
#define D__Mutate_                 x
#define D__name_color              x
#define D__name_font               x
#define D__obj_dude                x
#define D__old_fid1                x
#define D__old_str1                x
#define D__outlined_object         x
#define D__patches                 x
#define D__pc_name                 x
#define D__pc_proto                x
#define D__PeanutButter            x
#define D__perk_lev                x
#define D__proto_main_msg_file     x
#define D__ptable                  x
#define D__pud                     x
#define D__queue                   x
#define D__quick_done              x
#define D__RedColor                x
#define D__slot_cursor             x
#define D__sneak_working           x
#define D__stack                   x
#define D__stack_offset            0x19B6C4
#define D__sthreads                x
#define D__str                     x
#define D__Tag_                    x
#define D__tag_skill               x
#define D__target_curr_stack       0x19B82C
#define D__target_pud              0x19B824
#define D__target_stack            0x19B744
#define D__target_stack_offset     0x19B6EC
#define D__text_char_width         x
#define D__text_height             x
#define D__text_to_buf             x
#define D__text_width              x
#define D__ticker                  x
#define D__title_color             x
#define D__title_font              x

#endif //F1DPPATCHER_FALLOUTENGINE_H
