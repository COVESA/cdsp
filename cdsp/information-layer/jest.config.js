/** @type {import('ts-jest').JestConfigWithTsJest} **/
module.exports = {
  testMatch: ["**/*.test.ts"],
  testEnvironment: "node",
  transform: {
    "^.+.tsx?$": ["ts-jest",{}],
  },
};