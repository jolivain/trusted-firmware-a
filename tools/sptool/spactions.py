#!/usr/bin/python3
# Copyright (c) 2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
import json

# TODO:
# - use proper logging framework (VERBOSE, INFO, NOTICE and things)

def pretty_dict(ugly: dict) -> json:
    return json.dumps(ugly, indent=4)

def sp_logged_action(action):
    def logged_action(*args, **kwargs):
        try:
            layout = args[0]
            print(f"Calling {action.__name__};\n\nSP Layout node:\n{pretty_dict(layout)}\n\nCurrent Arguments: {pretty_dict(kwargs)}")
        except:
            print(f"Logging {action.__name__} failed. Both SP Layout and kwargs need to be json serializable.")
        return action(*args, **kwargs)
    return logged_action

def append_called(action, sp, **kwargs):
    kwargs["called"].append(f"{action.__name__} -> {sp}")
    return kwargs

class SpSetupActions:
    per_sp_actions = []
    global_actions = []

    def sp_action(in_action=None, global_action = False, log_calls=False):
        def append_action(action):
            action_name = action.__name__
            action = sp_logged_action(action) if log_calls else action
            action.__name__ = action_name

            if global_action:
                SpSetupActions.global_actions.append(action)
            else:
                SpSetupActions.per_sp_actions.append(action)
            return action

        if in_action is not None:
            return append_action(in_action)

        return append_action

    def _run_per_sp_actions(sp_layout={}, verbose = False, **kwargs):
        if verbose:
            print("\n-> Running per SP actions:\n")
        for sp in sp_layout.keys():
            for action in SpSetupActions.per_sp_actions:
                if verbose:
                    print(f"action: {action.__name__}, {sp}")
                kwargs = append_called(action, sp, **kwargs)
                kwargs = action(sp_layout, sp, **kwargs)
                if kwargs is None:
                    print(f"WARN: Kwargs is None in between action calls: {action.__name__}\n")
        return kwargs

    def _run_global_actions(sp_layout={}, verbose = False, **kwargs):
        if verbose:
            print("\n-> Running global actions:\n")

        for action in SpSetupActions.global_actions:
            if verbose:
                print(f"action: {action.__name__}")
            kwargs = append_called(action, "GLOBAL", **kwargs)
            kwargs = action(sp_layout, **kwargs)
            if kwargs is None:
                print(f"WARN: Kwargs is None in between action calls: {action.__name__}\n")
        return kwargs

    def run_actions(sp_layout={}, verbose=False, **kwargs):
        if verbose:
            print("---- Running executing all Sp Setup Actions ----")
        kwargs["called"] = []
        kwargs = SpSetupActions._run_global_actions(sp_layout, verbose, **kwargs)
        kwargs = SpSetupActions._run_per_sp_actions(sp_layout, verbose, **kwargs)
        if verbose:
            print("---- Finished executing all Sp Setup Actions ----")

if __name__ == "__main__":
    # Executing this module will have the following test code/playground executed
    sp_layout = {
        "cactus-primary" : {
            "boot-info": True,
            "image": {
                "file": "cactus.bin",
                "offset":"0x2000"
            },
            "pm": {
                "file": "cactus.dts",
                "offset":"0x1000"
            },
            "owner": "SiP"
        },

        "cactus-secondary" : {
            "image": "cactus.bin",
            "pm": "cactus-secondary.dts",
            "owner": "Plat"
        },

        "cactus-tertiary" : {
            "image": "cactus.bin",
            "pm": "cactus-tertiary.dts",
            "owner": "Plat"
        },

        "ivy" : {
            "image": "ivy.bin",
            "pm": "ivy.dts",
            "owner": "Plat"
        }
    }

    #Example of how to use this module
    @SpSetupActions.sp_action(global_action=True)
    def my_action1(sp_layout, **kwargs):
        print(f"inside function my_action1{sp_layout}")
        return kwargs

    @SpSetupActions.sp_action
    def my_action2(sp_layout, sp_name, **kwargs):
        print(f"inside function my_action2; SP: {sp_name} {sp_layout}")
        return kwargs

    SpSetupActions.run_actions(sp_layout)
