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

function tms_field_get_terms_list($vocabulary){
	module_load_include('inc', 'taxonomy_menu_sync');

	$settings = variable_get('taxonomy_menu_sync_list', array());
	uasort($settings, 'drupal_sort_weight');

	$terms = array();
	$menus = array();
	foreach($settings as $menu_name => $data){
		if($data['vocabulary'] != $vocabulary->machine_name){
			continue;
		}
		$menus[] = $menu_name;
	}
	$tids = _tms_field_get_first_level_terms($menus);
	foreach($tids as $tid){
		$term = taxonomy_term_load($tid);
		$term->bundle = taxonomy_menu_sync_get_term_bundle($term);
		$term->depth = 0;
		$terms[$tid] = $term;

		$children = taxonomy_get_tree($vocabulary->vid, $tid, null, true);
		foreach($children as $child){
			$child->bundle = taxonomy_menu_sync_get_term_bundle($child);
			$child->depth++;
			$terms[$child->tid] = $child;
		}
	}

	return $terms;
}

function _tms_field_get_first_level_terms($menus){
	$query = db_select('menu_links', 'ml');
	$query->join('taxonomy_menu_sync_relations', 'tr', 'ml.mlid = tr.mlid');
	$query->join('taxonomy_term_data', 'td', 'tr.tid = td.tid');

	$query
		->fields('td', array('tid'))
		->condition('depth', 1)
		->condition('menu_name', $menus);

	foreach($menus as $menu_name){
		$query->orderBy("ml.menu_name = '$menu_name'", 'DESC');
	}
	$query->orderBy('ml.weight', 'asc');
	$tids = $query->execute()->fetchCol();

	return $tids;
}