/* eslint-env node */

"use strict";

const config = require("./.cz.json");

module.exports = {
    extends: ["@commitlint/config-conventional"],
    rules: {
        "header-max-length": [2, "always", config.maxHeaderWidth],
        "body-max-line-length": [1, "always", config.maxLineWidth]
    }
};
