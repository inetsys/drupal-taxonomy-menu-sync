<?php

function taxonomy_menu_sync_menu_vocabularies_form($form, &$form_state){
	// Synchronized menus

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
		'#title' => t('Synchronized menus'),
		'#type' => 'fieldset',
		'#collapsible' => false,
		'#collapsed' => false,
		'#tree' => true,
	);

	foreach($menus as $key => $label){
		$form['taxonomy_menu_sync_list'][$key] = array(
			'#title' => $label,
			'#type' => 'select',
			'#options' => $options,
			'#default_value' => isset($defaults[$key]) ? $defaults[$key] : '',
		);
	}

	// Delete default *Placeholder* action

	$form['taxonomy_menu_sync_default_actions'] = array(
		'#type' => 'checkboxes',
		'#title' => t('Default item actions'),
		'#options' => array(
			'dummy' => t('Placeholder'),
			'tax_sync' => t('Add section'),
			'url' => t('URL'),
			'existing' => t('Existing content')
		),
		'#default_value' => variable_get('taxonomy_menu_sync_default_actions', array(
			'dummy' => false,
			'tax_sync' => 'tax_sync',
			'url' => 'url',
			'existing' => 'existing',
		))
	);

	return system_settings_form($form);
}