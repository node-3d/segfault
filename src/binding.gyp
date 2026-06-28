{
	'variables': {
		'arch': '<!(node -p "process.arch")',
	},
	'targets': [{
		'target_name': 'segfault',
		'includes': ['common.gypi'],
		'defines!': ['UNICODE', '_UNICODE'],
		'sources': [
			'cpp/bindings.cpp',
			'cpp/segfault-handler.cpp',
		],
		'include_dirs': [
			'<!@(node -e "import(\'@node-3d/addon-tools\').then((m) => m.printInclude())")',
		],
		'conditions': [
			['OS=="win"', {
				'sources': ['cpp/stack-windows.cpp'],
			}],
		],
	}],
}
