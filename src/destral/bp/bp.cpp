#include "bp.h"
#define SOL_ALL_SAFETIES_ON 1
#define SOL_PRINT_ERRORS 0
#define SOL_NO_EXCEPTIONS 1
#include "sol/state.hpp"
#include "ap/ap_debug.h"

namespace ds::bp {

	void set_bindings(sol::state& lua) {
	}

	int my_exception_handler(lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description) {
		// L is the lua state, which you can wrap in a state_view if necessary
		// maybe_exception will contain exception, if it exists
		// description will either be the what() of the exception or a description saying that we hit the general-case catch(...)
		if (maybe_exception) {
			const std::exception& ex = *maybe_exception;
			AP_WARNING("Lua exception: %s", ex.what());
		} else {
			AP_WARNING("Lua exception: %s", description.data());
		}

		// you must push 1 element onto the stack to be 
		// transported through as the error object in Lua
		// note that Lua -- and 99.5% of all Lua users and libraries -- expects a string
		// so we push a single string (in our case, the description of the error)
		return sol::stack::push(L, description);
	}


	// comentar al forum que aixo no funciona: https://sol2.readthedocs.io/en/latest/exceptions.html ... quina basura
	void my_panic(sol::optional<std::string> maybe_msg) {
		AP_WARNING("Lua is in a panic state and will now abort() the application");
		
		if (maybe_msg) {

			const std::string& msg = maybe_msg.value();
			AP_WARNING("Lua error msg: %s", msg.c_str());
			
		}
	
	}

	void init() {
		//sol::state lua;
		//lua.open_libraries(sol::lib::base);
		//lua.script("print('Hello from lua')");
		//sol::protected_function_result result = lua.safe_script("bad.code");
		//if (result.valid()) {
		//	// Call succeeded
		//	
		//	AP_TRACE("Call Succeeded");
		//} else {
		//	// Call failed
		//	sol::error err = result;
		//	std::string what = err.what();
		//	AP_TRACE("call failed, sol::error::what() is %s", what.c_str());
		//	
		//	// 'what' Should read 
		//	// "Handled this message: negative number detected"
		//}

		sol::state lua(sol::c_call<decltype(&my_panic), &my_panic>);

		lua.open_libraries(sol::lib::base);
//		lua.set_panic(sol::c_call<decltype(&my_panic), &my_panic>);
		lua.set_exception_handler(&my_exception_handler);
		// load and execute from string
		//lua.script();
		auto res = lua.load("boom_goes.the_dynamite");
		if (!res.valid()) {
			sol::error e = res;
			AP_TRACE("The code has failed to run! %s", e.what());
		}

		//auto bad_code_result = lua.script("123 herp.derp", [](lua_State*, sol::protected_function_result pfr) {
		//	// pfr will contain things that went wrong, for either loading or executing the script
		//	// Can throw your own custom error
		//	// You can also just return it, and let the call-site handle the error if necessary.
		//	sol::error err = pfr;
		//	AP_TRACE("The code has failed to run! %s", err.what());
		//	return pfr;
		//	});

		//auto result = lua.safe_script("bad.code");
		//try {
		//	
		//	

		//} catch (std::exception& e) {
		//	AP_TRACE("The code has failed to run! %s", e.what());

		//}
		/*if (!result.valid()) {
			sol::error err = result;
			AP_TRACE("The code has failed to run! %s", err.what());
		}*/


		
	}

	void shutdown() {

	}

}