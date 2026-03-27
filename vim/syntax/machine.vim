if exists('b:current_syntax')
  finish
endif

syntax keyword machineKeyword main func struct var const if else elif while print ret goto label switch case default module unsafe
syntax keyword machineBoolean true false
syntax keyword machineType i64 f64 hp str ptr list array bool void
syntax match machineDirective /^\s*bin\.runtime\>/
syntax match machineDirective /^\s*unsafe\.enable\>/
syntax match machineDirective /^\s*target\.[A-Za-z0-9_-]\+\>/
syntax match machineDirective /^\s*backend\.[A-Za-z0-9_-]\+\>/
syntax keyword machineBuiltin len hp sqrt sin cos pow hp_add hp_sub hp_mul hp_div hp_sqrt hp_pow
syntax keyword machineBuiltin alloc_bytes free_mem store_i64 load_i64 store_f64 load_f64 store_str load_str
syntax keyword machineBuiltin ptr_from_i64 ptr_to_i64 ptr_offset
syntax keyword machineBuiltin store_u8 store_u16 store_u32 store_u64 load_u8 load_u16 load_u32 load_u64
syntax keyword machineBuiltin volatile_store_u8 volatile_store_u16 volatile_store_u32 volatile_store_u64
syntax keyword machineBuiltin volatile_load_u8 volatile_load_u16 volatile_load_u32 volatile_load_u64
syntax keyword machineBuiltin syscall0 syscall1 syscall2 syscall3 syscall4 syscall5 syscall6
syntax keyword machineBuiltin mmap_anon mmap_anon_exec munmap_mem fd_open_ro fd_open_wo fd_open_rw fd_close fd_read fd_write fd_seek ioctl_i64
syntax keyword machineBuiltin asm_nop asm_pause asm_mfence asm_lfence asm_sfence asm_rdtsc cpu_in8 cpu_out8
syntax keyword machineBuiltin pmm_alloc_page pmm_alloc_pages pmm_total_bytes pmm_used_bytes page_identity_map_2m apic_supported apic_enable apic_eoi
syntax keyword machineBuiltin list_new list_push_back list_get list_size list_free
syntax keyword machineBuiltin array_new array_push array_get array_set array_len array_free
syntax keyword machineBuiltin grid_new grid_get grid_set grid_rows grid_cols grid_fill grid_free
syntax keyword machineBuiltin term_enable_raw term_disable_raw term_key_available term_read_key term_enable_mouse term_disable_mouse term_poll_event term_last_key
syntax keyword machineBuiltin term_mouse_x term_mouse_y term_mouse_button term_clear term_flush term_move_cursor term_hide_cursor term_show_cursor term_draw_text
syntax keyword machineBuiltin sleep_ms tick_ms timer_reset timer_elapsed_ms
syntax keyword machineBuiltin win_create win_destroy win_is_open win_poll_event win_last_key win_mouse_x win_mouse_y win_mouse_button win_clear win_present win_set_title
syntax keyword machineBuiltin win_draw_rect win_fill_rect win_draw_line win_draw_pixel win_draw_text
syntax keyword machineBuiltin image_load image_draw image_draw_scaled image_width image_height image_free video_play video_stop video_is_running addr index
syntax match machineComment /--.*/
syntax region machineString start=/"/ skip=/\\./ end=/"/
syntax match machineNumber /\<[0-9]\+\(\.[0-9]\+\)\?\>/
syntax match machineOperator /==/
syntax match machineOperator /!=/
syntax match machineOperator /<=/
syntax match machineOperator />=/
syntax match machineOperator /&&/
syntax match machineOperator /||/
syntax match machineOperator /[-+*\/%=<>@^.:,\[\]()]/
syntax match machineFunction /^[ \t]*func\s\+\zs[A-Za-z_][A-Za-z0-9_]*/
syntax match machineModuleName /^[ \t]*module\s\+\zs[A-Za-z_][A-Za-z0-9_]*/
syntax match machineTypeName /^[ \t]*struct\s\+\zs[A-Za-z_][A-Za-z0-9_]*/

highlight default link machineKeyword Keyword
highlight default link machineBoolean Boolean
highlight default link machineType Type
highlight default link machineDirective PreProc
highlight default link machineBuiltin Function
highlight default link machineComment Comment
highlight default link machineString String
highlight default link machineNumber Number
highlight default link machineOperator Operator
highlight default link machineFunction Function
highlight default link machineModuleName Identifier
highlight default link machineTypeName Type

let b:current_syntax = 'machine'
