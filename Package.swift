// swift-tools-version: 5.4
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
	name: "MultiMarkdown",
	platforms: [
		.iOS(.v12),
		.macOS(.v10_13),
		.tvOS(.v12),
		.watchOS(.v4)
	],
	products: [
		.library(
			name: "MultiMarkdown",
			targets: ["MultiMarkdown"])
	],
	targets: [
		.target(
			name: "MultiMarkdown",
			path: "Sources",
			publicHeadersPath: "Public",
			cSettings: [
				.headerSearchPath("Private/"),
				.headerSearchPath("Public/"),
				.define("YY_CTX_LOCAL")
			]
		),
	]
)
