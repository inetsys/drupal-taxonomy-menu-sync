<?php

/**
 * @param object $new_term
 * @param object $old_term
 */
function hook_taxonomy_term_sync_modified($new_term, $old_term){
	$query = new EntityFieldQuery();
	$query->entityCondition('entity_type', 'node');
	$query->fieldCondition('FIELD_NAME', 'tid', $old_term->tid);
	$result = $query->execute();

	if(isset($result['node'])){
		$nodes = entity_load('node', array_keys($result['node']));

		foreach($nodes as $node){
			// Update each node with new taxonomy term tid
			// ...
		}
	}
}

/**
 * @param array $item
 * @param object $term
 * @param array $bundle
 * @return array
 */
function hook_menu_item_element_bundle_operations($item, $term, $bundle){
	return array(
		'add_page' => array(
			'#type' => 'link',
			'#title' => t('add new content'),
			'#href' => 'node/add/page',
			'#options' => array(
				'query' => array(
					'field_taxonomy_term' => $term->tid
				),
			),
			'weight' => 20,
		)
	);
}