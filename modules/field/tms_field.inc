<?php

function tms_field_get_bundle_options(){
	module_load_include('inc', 'taxonomy_menu_sync');
	$options = array();

	$vocabularies = taxonomy_get_vocabularies();
	foreach($vocabularies as $vocabulary){
		$vocabulary_bundles = variable_get('taxonomy_menu_sync_bundles_'.$vocabulary->machine_name, array());
		foreach($vocabulary_bundles as $bundle){
			$options[$vocabulary->machine_name.'|'.$bundle['machine_name']] = $vocabulary->name.': '.$bundle['name'];
		}
	}

	return $options;
}