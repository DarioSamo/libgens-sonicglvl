/*
TO DO:
-fixing default values menu
-fixing calculate good proportion
-fixing slow down in calculate good proportion
*/

lightmapPath = undefined


--This function checks whether a new unwrap should be created for channel 2 and does so if necessary.
--	Param obj: the object to work on.
--	Param createUnwrap: whether to add a new unwrap modifier if an unwrap existed already.
--	Param collapseAfterUnwrap: whether to collapse the modifier-stack after adding the unwrap modifier.
function handleUnwrapping obj createUnwrap collapseAfterUnwrap =
(
	--Check whether an unwrap in channel 2 already exists.
	addModifier obj (edit_mesh())
	local channel2UnwrapExists = meshop.getMapSupport obj 2
	deleteModifier obj 1

	--Create a new unwrap in channel 2 if requested or if it does not exist yet.
	if createUnwrap OR NOT channel2UnwrapExists then
	(
		--Add the unwrap-modifier.
		addModifier obj (unwrap_UVW())

		--The unwrap is put in channel 2, so as not to disturb the unwrap for the diffuse texture.
		obj.modifiers[1].setMapChannel 2

		--This is a bit of a hack, used to force 3dsmax to update the modifier dialogue. This is
		--necessary for flattenMap to work.
		classOf obj
		
		--Select all textureTriangles, to apply the flattenMap on.
		selectionBitArray = #{}
		for i = 1 to obj.modifiers[1].numberPolygons() do
		(
			append selectionBitArray i
		)
		obj.modifiers[1].selectPolygons selectionBitArray

		--Generate an automatic unwrap.
		obj.modifiers[1].flattenMap 50 #([1,0,0], [-1,0,0], [0,1,0], [0,-1,0], [0,0,1], [0,0,-1]) 0.01 true 0 true true

		--Collapse the modifier stack (including the unwrap modifier) if requested.
		if collapseAfterUnwrap then
		(
			collapseStack obj
		)
	)
)

--The function that actually performs the generation of the lightmaps.
--	Param proportionMultiplier: a multiplier that can be used to alter the proportion between resolution and scale of the scene.
--	Param forceAllToMaximumResolution: used to turn of adaptive resolution calculation for the texture.
--	Param createUnwrap: whether to add a new unwrap modifier.
--	Param collapseAfterUnwrap: whether to collapse the modifier-stack after adding the unwrap modifier.
--	Param highestResolutionPower: the highest resolution lightmap a large object can get.
function generateLightMaps resolutionMode createUnwrap collapseAfterUnwrap highestResolutionPower appendix_id =
(
	max modify mode
	
	renderFileNames = #()
	renderFileSizes = #()
	renderFileSize = 0
	
	if (resolutionMode == 1) then (
		renderfilePath = getOpenFileName caption:"Select the render file to read from" types:"Render List (*.txt)|*.txt"
		if (renderfilePath == undefined) then return 0
		
		renderFile = openFile renderfilePath
		if renderFile != undefined then (
			while not eof renderFile do (
				renderFileSize += 1
				renderLine = readLine renderFile
				renderTokens = filterString renderLine " " splitEmptyTokens:true
				renderFileSizes[renderFileSize] = renderTokens[1] as integer
				renderFileNames[renderFileSize] = renderTokens[2] as string
			)
			
			close renderFile
		)
	)
	
	lightmapPath = getSavePath caption:"Select the folder to save to"
	if (lightmapPath == undefined) then return 0

	--This will be reused for all lightmaps to be generated.
	local map = VRayLightingMap()

	--While having its lightmap generated, each object temporarily gets this white material.
	--This is necessary, because VRay does not fully support texture baking and cannot ignore
	--the standard material.
	local whiteMaterial = standardMaterial()
	whiteMaterial.diffuse = color 255 255 255

	--These are the objects to be processed.
	local selectedObjects = copy ($selection as array) #noMap
	
	--Process every object.
	for obj in selectedObjects do
	(
		--abort operation if escape is pressed
		if keyboard.escPressed then
		(
			exit
		)

		--Only generate a lightmap if the object is geometry.
		if isKindOf obj GeometryClass AND NOT isKindOf obj TargetObject then
		(
			--Select the object.
			select obj

			--Create an unwrap in channel 2 if requested or necessary.
			handleUnwrapping obj createUnwrap collapseAfterUnwrap
			
			--Calculate the resolution in pixels to be used for this object.
			local resolutionInPixels = pow 2 highestResolutionPower
			for RenderItem = 1 to renderFileSize do (
				if (obj.name == renderFileNames[RenderItem]) then (
					resolutionInPixels = renderFileSizes[RenderItem]
					exit
				)
			)

			--Change the material to the white material and store the original material.
			local originalMaterial = obj.material
			if (appendix_id != 3) then obj.material = whiteMaterial

			--Settings for the baking of this object.
			map.autoSzOn = false
			--map.directOn = true
			map.elementName = "baked"
			map.enabled = true
			
			if (appendix_id == 1) then appendix = "_lightmap"
			if (appendix_id == 2) then appendix = "_shadowmap"
			if (appendix_id == 3) then appendix = "_completemap"
			
			map.filename = (lightmapPath + "\\" + obj.name + appendix + ".png")
			map.filenameUnique = false
			map.filetype = "PNG"
			map.filterOn = true
			--map.inDirectOn = true
			map.outputSzX = resolutionInPixels
			map.outputSzY = resolutionInPixels
			--map.shadowsOn = true
			map.targetMapSlotName = ""

			--Attach the baking to this object.
			local bakeobject = obj.INodeBakeProperties
			bakeobject.removeAllBakeElements()
			bakeobject.addBakeElement map
			bakeobject.nDilations = 64
			bakeObject.bakeEnabled = true
			bakeObject.bakeChannel = 2

			--let the currently selected object be baked.
			render rendertype:#BakeSelected outputwidth:map.outputSzX outputheight:map.outputSzY vfb:true outputfile:map.filename
			
			--Baking is finished for this object, so reset the baking settings for this object.
			bakeobject.removeAllBakeElements()

			--Restore the original material of this object.
			obj.material = originalMaterial
		)
	)
	
	--Recover the selection the user made before calling this script.
	select selectedObjects
)





--The user interface for the lightmap generator.
rollout lightmapGenerator "Lightmap Generator"
(
	Group "Resolution:"
	(
		RadioButtons resolutionModeRadioButtons "Resolution Mode:" labels:#("Render File", "Manual") default:1
		RadioButtons resolutionRadioButtons "Resolution:" labels:#("32", "64", "128", "256", "512", "1024", "2048", "4096") default:5
		RadioButtons texnameRadioButtons "Texture Appendix:" labels:#("_lightmap", "_shadowmap","_completemap") default:1
	)
	Group "Channel 2 unwrap"
	(
		CheckBox unwrapCheckBox "Create new unwrap" checked:false
		CheckBox collapseCheckBox "Collapse modifiers" checked:false enabled:false
	)
	Button executeButton "Start lightmap generation"
	
	on unwrapCheckBox changed newState do
	(
		collapseCheckBox.enabled = newState
	)
	on executeButton pressed do
	(
		generateLightMaps resolutionModeRadioButtons.state \
						  unwrapCheckBox.checked \
						  collapseCheckBox.checked \
						  (resolutionRadioButtons.state + 4) \
						  texnameRadioButtons.state
	)
)

--Put the user interface on the screen.
createDialog lightmapGenerator