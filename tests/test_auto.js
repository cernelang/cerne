/*
    Cerne Compiler - automated test runner

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
const   fs = require("fs"),
        path = require("path"),
        { exec } = require("child_process");

// currently, we're on the test folder, so tests are run on ./tests
const test_dir = "./tests";

// the executable should be in the root directory, so we can run it with ./cerne
const cerne_executable = "../cerne";

/**
 * Utility function to run a list of test files with the Cerne Compiler
 * @param {Array<string>} tests a list of all test file paths to run
 * @returns {Promise<boolean>} a promise that, when resolved, returns whether all tests passed with the expected output
 */
function run(tests) {
    return new Promise((resolve, reject) => {
        let all_passed = true;

        // run each test and match output with expected output (if current test file ends in _fail.ce, output json should have errors, otherwise it should include 0 errors)
        for(const test of tests) {
            // expected output (false means errors are expected, true means no errors are expected)
            const expected = test.endsWith("_fail.ce") ? false : true;

            // run the test with the cerne executable and capture output
            exec(`${cerne_executable} ${test} --dump=ast`, (err, stdout, stderr) => {
                if(err) {
                    console.error(`[cerne] [test]: Error running test ${test}:`, err);
                    all_passed = false;
                    return;
                }

                /*
                    now we check if the output actually matches
                    when dumping, cerne's compiler outputs a JSON with the AST and error/warning counts
                    this JSON is stored in a file with the same name along with .ast.json extension
                    so we can read that file and immediately parse and check the error count
                */
                const output_path = `${test}.ast.json`;
                fs.readFile(output_path, "utf-8", (err, data) => {
                    if(err) {
                        console.error(`[cerne] [test]: Error reading output file for test ${test}:`, err);
                        all_passed = false;
                        return;
                    }
                    
                    // try to parse the JSON
                    try {
                        const output = JSON.parse(data);
                        const has_errors = output.errors > 0;
                        if(has_errors !== expected) {
                            console.error(`[cerne] [test]: Test ${test} failed. Expected errors: ${!expected}, Actual errors: ${has_errors}`);
                            all_passed = false;
                        }
                    } catch(parse_err) {
                        console.error(`[cerne] [test]: Error parsing output JSON for test ${test}:`, parse_err);
                        all_passed = false;
                    }

                });

            })
        }

        // returns whether all tests have passed with the expected output
        return all_passed;
    });
}

// actually run all tests in the test directory
fs.readdir(test_dir, (err, files) => {
    // error reading the actual directory
    if(err) {
        console.error("[cerne] [test]: Error reading test directory:", err);
        return;
    }

    // filter for .ce files
    const test_files = files.filter(file => path.extname(file) === ".ce").map(file => path.join(test_dir, file));

    // test runner
    run(test_files).then(success => {
        if(success) {
            console.log("[cerne] [test]: Output was as expected for all tests.");
        } else {
            console.error("[cerne] [test]: Output did not match expected output for some tests. Check the logs above for details.");
        }
    }).catch(err => {
        console.error("[cerne] [test]: Error running tests:", err);
    });
});