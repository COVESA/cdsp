// @ts-check

import eslint from "@eslint/js";
import pluginPrettier from "eslint-plugin-prettier";
import prettierConfig from "eslint-config-prettier";
import tseslint, { parser } from "typescript-eslint";

export default tseslint.config(
  {
    ignores: ["coverage/**", "**/dist/**", "node_modules/**"],
  },
  {
    files: ["./**/*.{ts,tsx}"],
    extends: [eslint.configs.recommended, ...tseslint.configs.strict],
    plugins: {
      prettier: pluginPrettier,
      //"@typescript-eslint": ESLintPlugin,
    },
    languageOptions: {
      parser: parser,
      parserOptions: {
        project: "./tsconfig.json", // Ensure it points to your tsconfig
      },
    },
    rules: {
      "max-depth": ["error", 3],
      "@/no-console": "error",
      "@typescript-eslint/no-unnecessary-condition": "error",
      "@typescript-eslint/no-floating-promises": "error",
      "no-shadow": "off",
      "@typescript-eslint/no-shadow": "error",
      "@typescript-eslint/naming-convention": "error",
      "@no-nested-ternary": "warn",
      "prettier/prettier": "error",
      ...prettierConfig.rules,
    },
  }
);
