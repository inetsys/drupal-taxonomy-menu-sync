<?php

function taxonomy_menu_sync_menu_vocabularies_form($form, &$form_state){
	$defaults = variable_get('taxonomy_menu_sync_list', array());

	$menus = menu_get_menus();
	$vocabularies = taxonomy_get_vocabularies();
	$options = array(0 => '-');
	foreach($vocabularies as $item){
		$key = $item->machine_name;
		$value = $item->name;
		$options[$key] = $value;
	}

	$form['taxonomy_menu_sync_list'] = array(
		'#tree' => true
	);

	foreach($menus as $key => $label){
		$form['taxonomy_menu_sync_list'][$key] = array(
			'#title' => $label,
			'#type' => 'select',
			'#options' => $options,
			'#default_value' => isset($defaults[$key]) ? $defaults[$key] : '',
		);
	}

	return system_settings_form($form);
}