# Destral

Programming Info:

To set the default working directories for subprojects:
https://stackoverflow.com/questions/41864259/how-to-set-working-directory-for-visual-studio-2017-rc-cmake-project




############ STRUCTS ################

Always initialize all structures with = {0}

struct ap_dbg_info {
}

struct ds_app_desc {
}

ds -> global namespace / library
app -> module namespace
desc -> data description

############# Functions #############

void ap_dbg_init(int width, int height, int* out_multi) {
}

void ds_app_init(ds_app_desc * desc) {
	int local_variable = 0;
}

ds -> global namespace / library
app -> module namespace
init -> operation

############# Functions #############

enum ap_dbg_loglvl {
    AP_DBG_TRACE = 0,
    AP_DBG_INFO,
    AP_DBG_WARNING,
    AP_DBG_ERROR
};

ap -> global namespace / library
dbg -> module namespace
loglvl -> enum desccription