<?php

function taxonomy_menu_sync_settings_form($form, &$form_state){
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
			'#type' => 'fieldset',
			'#title' => $label,
			'#collapsible' => true,
			'#collapsed' => !isset($defaults[$key]['vocabulary']) || !$defaults[$key]['vocabulary'],
		);
		// Vocabulary
		$form['taxonomy_menu_sync_list'][$key]['vocabulary'] = array(
			'#title' => t('Vocabulary'),
			'#type' => 'select',
			'#options' => $options,
			'#default_value' => isset($defaults[$key]['vocabulary']) ? $defaults[$key]['vocabulary'] : '',
		);
		// Weight
		$form['taxonomy_menu_sync_list'][$key]['weight'] = array(
			'#type' => 'weight',
			'#title' => t('Weight'),
			'#default_value' => isset($defaults[$key]['weight']) ? $defaults[$key]['weight'] : '',
			'#states' => array(
				'invisible' => array(
					'select[name="taxonomy_menu_sync_list['.$key.'][vocabulary]"]' => array(
						'value' => 0
					)
				)
			)
		);
	}

	// Delete default actions from content_menu module

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

function taxonomy_menu_sync_vocabulary_bundles($vocabulary){
	drupal_add_css(drupal_get_path('module', 'advanced_content').'/advanced_content.admin.css', array('weight' => 10));

	$bundles = variable_get('taxonomy_menu_sync_bundles_'.$vocabulary->machine_name, array());

	$field_instances = field_info_instances('taxonomy_term', $vocabulary->machine_name);
	// return '<pre>'.print_r($field_instances, true).'</pre>';

	$header = array(
		t('Name'),
		t('Field'),
		t('Value'),
		t('Options'),
	);
	$rows = array();
	foreach($bundles as $key => $data){
		$row = array();

		$field_instance = $field_instances[$data['field']];
		$row[] = $data['name'];
		$row[] = $field_instance['label'];
		$row[] = $data['value'];

		$operations = array();
		$operations['edit'] = array(
			'title' => t('edit'),
			'href' => 'admin/structure/taxonomy/'.str_replace('_', '-', $vocabulary->machine_name).'/bundles/'.str_replace('_', '-', $data['machine_name']),
			'query' => array(
				'destination' => 'admin/structure/taxonomy/'.str_replace('_', '-', $vocabulary->machine_name).'/bundles',
			),
		);
		$operations['fields'] = array(
			'title' => t('manage fields'),
			'href' => 'admin/structure/taxonomy/'.str_replace('_', '-', $vocabulary->machine_name).'/bundles/'.str_replace('_', '-', $data['machine_name']).'/fields',
			'query' => array(
				'destination' => 'admin/structure/taxonomy/'.str_replace('_', '-', $vocabulary->machine_name).'/bundles',
			),
		);
		$operations['delete'] = array(
			'title' => t('delete'),
			'href' => 'admin/structure/taxonomy/'.str_replace('_', '-', $vocabulary->machine_name).'/bundles/'.str_replace('_', '-', $data['machine_name']).'/delete',
			'query' => array(
				'destination' => 'admin/structure/taxonomy/'.str_replace('_', '-', $vocabulary->machine_name).'/bundles',
			),
		);
		// Render an unordered list of operations links.
		$row[] = array(
			'data' => array(
				'#theme' => module_exists('ctools') ? 'links__ctools_dropbutton' : 'links__node_operations',
				'#links' => $operations
			),
		);

		$rows[] = $row;
	}

	return theme('table', array('header' => $header, 'rows' => $rows));
}

function taxonomy_menu_sync_vocabulary_add_bundle($form, &$form_state, $vocabulary){
	$form['vocabulary'] = array(
		'#type' => 'value',
		'#value' => $vocabulary
	);

	$form['machine_name'] = array(
		'#title' => t('Machine name'),
		'#type' => 'textfield',
		'#required' => true,
	);

	$form['name'] = array(
		'#title' => t('Name'),
		'#type' => 'textfield',
		'#required' => true,
	);

	$form['filter'] = array(
		'#title' => t('Bundle filter'),
		'#type' => 'fieldset',
		'#collapsible' => false,
		'#collapsed' => false,
	);
	$field_instances = field_info_instances('taxonomy_term', $vocabulary->machine_name);
	$options = array();
	foreach($field_instances as $key => $data){
		$options[$key] = $data['label'];
	}
	$form['filter']['field'] = array(
		'#title' => t('Field'),
		'#type' => 'select',
		'#required' => true,
		'#options' => $options
	);
	$form['filter']['value'] = array(
		'#title' => t('Value'),
		'#type' => 'textfield',
		'#required' => true,
	);

	$form['actions'] = array(
		'#type' => 'actions',
	);
	$form['actions']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save')
	);

	return $form;
}
function taxonomy_menu_sync_vocabulary_add_bundle_validate($form, &$form_state){
	$vocabulary = $form_state['values']['vocabulary'];
	$bundles = variable_get('taxonomy_menu_sync_bundles_'.$vocabulary->machine_name, array());

	if(isset($bundles[$form_state['values']['machine_name']])){
		form_set_error('machine_name', t('The machine name is already used.'));
	}
	if(!preg_match('/[a-z_]+/', $form_state['values']['machine_name'])){
		form_set_error('machine_name', t('Machine name must only contains letters and underscores'));
	}
}
function taxonomy_menu_sync_vocabulary_add_bundle_submit($form, &$form_state){
	$vocabulary = $form_state['values']['vocabulary'];
	$bundles = variable_get('taxonomy_menu_sync_bundles_'.$vocabulary->machine_name, array());

	$values = $form_state['values'];
	foreach(array(
		'vocabulary',
		'submit',
		'form_build_id',
		'form_token',
		'form_id',
		'op',
	) as $system_field){
		unset($values[$system_field]);
	}
	$values['vocabulary'] = $vocabulary->machine_name;
	$bundles[$values['machine_name']] = $values;

	variable_set('taxonomy_menu_sync_bundles_'.$vocabulary->machine_name, $bundles);

	drupal_set_message('Bundle saved');
}

function taxonomy_menu_sync_vocabulary_edit_bundle($form, &$form_state, $vocabulary, $bundle_name){
	module_load_include('inc', 'taxonomy_menu_sync');

	$bundle = taxonomy_menu_sync_load_bundle($vocabulary, $bundle_name);
	if(!$bundle){
		drupal_not_found();
		exit;
	}

	$form['vocabulary'] = array(
		'#type' => 'value',
		'#value' => $vocabulary
	);
	$form['bundle'] = array(
		'#type' => 'value',
		'#value' => $bundle
	);

	$form['machine_name'] = array(
		'#title' => t('Machine name'),
		'#type' => 'textfield',
		'#required' => true,
		'#disabled' => true,
		'#default_value' => $bundle['machine_name'],
	);

	$form['name'] = array(
		'#title' => t('Name'),
		'#type' => 'textfield',
		'#required' => true,
		'#default_value' => $bundle['name'],
	);

	$form['filter'] = array(
		'#title' => t('Bundle filter'),
		'#type' => 'fieldset',
		'#collapsible' => false,
		'#collapsed' => false,
	);
	$field_instances = field_info_instances('taxonomy_term', $vocabulary->machine_name);
	$options = array();
	foreach($field_instances as $key => $data){
		$options[$key] = $data['label'];
	}
	$form['filter']['field'] = array(
		'#title' => t('Field'),
		'#type' => 'select',
		'#required' => true,
		'#options' => $options,
		'#default_value' => $bundle['field'],
	);
	$form['filter']['value'] = array(
		'#title' => t('Value'),
		'#type' => 'textfield',
		'#required' => true,
		'#default_value' => $bundle['value'],
	);

	$form['actions'] = array(
		'#type' => 'actions',
	);
	$form['actions']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save')
	);

	return $form;
}
function taxonomy_menu_sync_vocabulary_edit_bundle_submit($form, &$form_state){
	$vocabulary = $form_state['values']['vocabulary'];
	$bundle = $form_state['values']['bundle'];
	$bundles = variable_get('taxonomy_menu_sync_bundles_'.$vocabulary->machine_name, array());

	$values = $form_state['values'];
	foreach(array(
		'vocabulary',
		'bundle',
		'submit',
		'form_build_id',
		'form_token',
		'form_id',
		'op',
	) as $system_field){
		unset($values[$system_field]);
	}
	$values['vocabulary'] = $vocabulary->machine_name;
	$bundles[$bundle['machine_name']] = $values;

	variable_set('taxonomy_menu_sync_bundles_'.$vocabulary->machine_name, $bundles);

	drupal_set_message('Bundle saved');
}

function taxonomy_menu_sync_vocabulary_manage_bundle_fields($form, &$form_state, $vocabulary, $bundle_name){
	module_load_include('inc', 'taxonomy_menu_sync');

	$bundle = taxonomy_menu_sync_load_bundle($vocabulary, $bundle_name);
	if(!$bundle){
		drupal_not_found();
		exit;
	}

	$defaults = variable_get('taxonomy_menu_sync_'.$vocabulary->machine_name.'__'.$bundle_name.'_data', array());
	$field_instances = field_info_instances('taxonomy_term', $vocabulary->machine_name);

	$form['vocabulary'] = array(
		'#type' => 'value',
		'#value' => $vocabulary
	);
	$form['bundle'] = array(
		'#type' => 'value',
		'#value' => $bundle
	);

	$form['filter'] = array(
		'#title' => t('Bundle filter: %field', array('%field' => $field_instances[$bundle['field']]['label'])),
		'#type' => 'textfield',
		'#disabled' => true,
		'#required' => true,
		'#default_value' => $bundle['value'],
	);

	$form['fields'] = array(
		'#type' => 'fieldset',
		'#title' => t('Fields'),
		'#collapsible' => false,
		'#collapsed' => false,
	);
	foreach($field_instances as $field_name => $field_data){
		if($field_name == $bundle['field']){
			continue;
		}

		$field = field_info_field_by_id($field_data['field_id']);

		$form['fields'][$field_name] = array(
			'#type' => 'fieldset',
			'#title' => $field_data['label'],
			'#collapsible' => true,
			'#collapsed' => isset($defaults[$field_name]['mode']) && $defaults[$field_name]['mode'] == 0 && isset($defaults[$field_name]['default_value']) && empty($defaults[$field_name]['default_value']),
			'#tree' => true,
		);

		$form['fields'][$field_name]['mode'] = array(
			'#title' => t('Mode'),
			'#type' => 'radios',
			'#options' => array(
				0 => t('Hidden'),
				1 => t('Visible'),
				2 => t('Required')
			),
			'#default_value' => isset($defaults[$field_name]['mode']) ? $defaults[$field_name]['mode'] : 1,
		);
		switch($field['type']){
			case 'list_boolean':
				$form['fields'][$field_name]['default_value'] = array(
					'#title' => t('Default value'),
					'#type' => 'checkbox',
					'#return_value' => $field['settings']['allowed_values'][1],
					'#default_value' => isset($defaults[$field_name]['default_value']) ? $defaults[$field_name]['default_value'] : $field_data['default_value'][0]['value'],
				);
				break;

			case 'list_integer':
				$form['fields'][$field_name]['default_value'] = array(
					'#title' => t('Default value'),
					'#type' => 'select',
					'#options' => $field['settings']['allowed_values'],
					'#empty_value' => '',
					'#default_value' => isset($defaults[$field_name]['default_value']) ? $defaults[$field_name]['default_value'] : $field_data['default_value'][0]['value'],
				);
				// drupal_set_message('<pre>'.print_r($field, true).'</pre>');
				// drupal_set_message('<pre>'.print_r($field_data, true).'</pre>');
				break;

			default:
				$form['fields'][$field_name]['default_value'] = array(
					'#title' => t('Default value'),
					'#type' => 'textfield',
					'#default_value' => isset($defaults[$field_name]['default_value']) ? $defaults[$field_name]['default_value'] : '',
				);
				break;
		}
	}

	$form['actions'] = array(
		'#type' => 'actions',
	);
	$form['actions']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save')
	);

	return $form;
}
function taxonomy_menu_sync_vocabulary_manage_bundle_fields_submit($form, &$form_state){
	$vocabulary = $form_state['values']['vocabulary'];
	$bundle = $form_state['values']['bundle'];

	$defaults = array();
	foreach($form_state['values'] as $field => $data){
		if(strpos($field, 'field_') !== 0){
			continue;
		}

		$defaults[$field] = $data;
	}
	variable_set('taxonomy_menu_sync_'.$vocabulary->machine_name.'__'.$bundle['machine_name'].'_data', $defaults);

	drupal_set_message('The configuration has been saved');
}

function taxonomy_menu_sync_vocabulary_delete_bundle($form, &$form_state, $vocabulary, $bundle_name){
	module_load_include('inc', 'taxonomy_menu_sync');

	$bundle = taxonomy_menu_sync_load_bundle($vocabulary, $bundle_name);
	if(!$bundle){
		drupal_not_found();
		exit;
	}

	$form['vocabulary'] = array(
		'#type' => 'value',
		'#value' => $vocabulary
	);
	$form['bundle'] = array(
		'#type' => 'value',
		'#value' => $bundle
	);

	return confirm_form($form, t('Do you really want to delete bundle %bundle?', array('%bundle' => $bundle['name'])), 'admin/structure/taxonomy/'.$vocabulary->machine_name.'/bundles');
}
function taxonomy_menu_sync_vocabulary_delete_bundle_submit($form, &$form_state){
	$vocabulary = $form_state['values']['vocabulary'];
	$bundle = $form_state['values']['bundle'];

	$bundles = variable_get('taxonomy_menu_sync_bundles_'.$vocabulary->machine_name, array());
	unset($bundles[$bundle['machine_name']]);
	variable_set('taxonomy_menu_sync_bundles_'.$vocabulary->machine_name, $bundles);

	variable_del('taxonomy_menu_sync_'.$vocabulary->machine_name.'__'.$bundle['machine_name'].'_data');

	$form_state['redirect'] = 'admin/structure/taxonomy/'.$vocabulary->machine_name.'/bundles';
	drupal_set_message(t('Taxonomy bundle %bundle has been deleted', array('%bundle' => $bundle['name'])));
}
