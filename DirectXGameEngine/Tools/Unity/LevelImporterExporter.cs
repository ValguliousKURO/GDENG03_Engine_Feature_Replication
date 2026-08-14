#if UNITY_EDITOR
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using UnityEditor;
using UnityEngine;

public sealed class LevelImporterExporter : EditorWindow
{
    [Serializable]
    private sealed class LevelFile
    {
        public int version = 1;
        public List<LevelObject> objects = new List<LevelObject>();
    }

    [Serializable]
    private sealed class LevelObject
    {
        public int id;
        public int parentId;
        public string name;
        public string type;
        public string texture;
        public bool enabled = true;
        public bool hasRigidBody;
        public bool physicsEnabled;
        public bool rigidBodyEnabled;
        public string rigidBodyType;
        public string rigidBodyCollider;
        public Vector3 rigidBodyColliderSize;
        public float rigidBodyMass;
        public bool rigidBodyUseGravity;
        public float pointLightIntensity;
        public float pointLightRange;
        public Vector3 position;
        public Vector3 rotation;
        public Vector3 scale = Vector3.one;
    }

    [MenuItem("Tools/Level Format/Import .level")]
    private static void ImportLevelMenu()
    {
        string path = EditorUtility.OpenFilePanel("Import .level", Application.dataPath, "level,json");
        if (string.IsNullOrEmpty(path)) return;

        ImportLevel(path);
    }

    [MenuItem("Tools/Level Format/Export .level")]
    private static void ExportLevelMenu()
    {
        string path = EditorUtility.SaveFilePanel("Export .level", Application.dataPath, "unity_scene.level", "level");
        if (string.IsNullOrEmpty(path)) return;

        ExportLevel(path);
    }

    private static void ImportLevel(string path)
    {
        string json = File.ReadAllText(path);
        LevelFile level = ParseLevel(json);
        if (level == null || level.objects == null)
        {
            Debug.LogError($"Invalid level file: {path}");
            return;
        }

        GameObject root = new GameObject(Path.GetFileNameWithoutExtension(path));
        Dictionary<int, GameObject> createdObjects = new Dictionary<int, GameObject>();

        foreach (LevelObject data in level.objects)
        {
            GameObject gameObject = CreateObject(data);
            if (!gameObject) continue;

            gameObject.name = string.IsNullOrWhiteSpace(data.name) ? data.type : data.name;
            gameObject.SetActive(data.enabled);
            createdObjects[data.id] = gameObject;
            gameObject.transform.SetParent(root.transform, false);
        }

        foreach (LevelObject data in level.objects)
        {
            if (!createdObjects.TryGetValue(data.id, out GameObject gameObject)) continue;

            Transform parent = root.transform;
            if (data.parentId != 0 && createdObjects.TryGetValue(data.parentId, out GameObject parentObject))
            {
                parent = parentObject.transform;
            }

            gameObject.transform.SetParent(parent, false);
            gameObject.transform.localPosition = data.position;
            gameObject.transform.localEulerAngles = data.rotation;
            gameObject.transform.localScale = data.scale == Vector3.zero ? Vector3.one : data.scale;
        }

        Selection.activeGameObject = root;
        Debug.Log($"Imported {createdObjects.Count} objects from {path}");
    }

    private static GameObject CreateObject(LevelObject data)
    {
        string type = data.type ?? string.Empty;
        GameObject gameObject;

        if (type.Equals("Plane", StringComparison.OrdinalIgnoreCase))
        {
            gameObject = GameObject.CreatePrimitive(PrimitiveType.Plane);
        }
        else if (type.Equals("Cube", StringComparison.OrdinalIgnoreCase) || type.Equals("Physics-Cube", StringComparison.OrdinalIgnoreCase))
        {
            gameObject = GameObject.CreatePrimitive(PrimitiveType.Cube);
        }
        else if (type.Equals("Sphere", StringComparison.OrdinalIgnoreCase) || type.Equals("Physics-Sphere", StringComparison.OrdinalIgnoreCase))
        {
            gameObject = GameObject.CreatePrimitive(PrimitiveType.Sphere);
        }
        else if (type.Equals("Capsule", StringComparison.OrdinalIgnoreCase) || type.Equals("Physics-Capsule", StringComparison.OrdinalIgnoreCase))
        {
            gameObject = GameObject.CreatePrimitive(PrimitiveType.Capsule);
        }
        else if (type.Equals("Cylinder", StringComparison.OrdinalIgnoreCase) || type.Equals("Physics-Cylinder", StringComparison.OrdinalIgnoreCase))
        {
            gameObject = GameObject.CreatePrimitive(PrimitiveType.Cylinder);
        }
        else if (type.Equals("Point Light", StringComparison.OrdinalIgnoreCase))
        {
            gameObject = new GameObject("Point Light");
            Light light = gameObject.AddComponent<Light>();
            light.type = LightType.Point;
            light.intensity = data.pointLightIntensity > 0.0f ? data.pointLightIntensity : 1.0f;
            light.range = data.pointLightRange > 0.0f ? data.pointLightRange : 10.0f;
        }
        else if (type.StartsWith("Obj:", StringComparison.OrdinalIgnoreCase))
        {
            gameObject = CreateModelInstance(type.Substring("Obj:".Length));
        }
        else
        {
            gameObject = new GameObject(string.IsNullOrWhiteSpace(type) ? "GameObject" : type);
        }

        ApplyRigidBody(gameObject, data);
        return gameObject;
    }

    private static GameObject CreateModelInstance(string modelName)
    {
        string[] guids = AssetDatabase.FindAssets($"{modelName} t:GameObject");
        if (guids.Length > 0)
        {
            string assetPath = AssetDatabase.GUIDToAssetPath(guids[0]);
            GameObject asset = AssetDatabase.LoadAssetAtPath<GameObject>(assetPath);
            if (asset)
            {
                return (GameObject)PrefabUtility.InstantiatePrefab(asset);
            }
        }

        Debug.LogWarning($"Model asset not found for Obj:{modelName}. Creating empty placeholder.");
        return new GameObject($"Obj:{modelName}");
    }

    private static void ApplyRigidBody(GameObject gameObject, LevelObject data)
    {
        bool shouldHaveRigidBody = data.hasRigidBody || data.physicsEnabled || data.rigidBodyEnabled;
        if (!shouldHaveRigidBody) return;

        Rigidbody rigidbody = gameObject.GetComponent<Rigidbody>();
        if (!rigidbody) rigidbody = gameObject.AddComponent<Rigidbody>();

        rigidbody.mass = data.rigidBodyMass > 0.0f ? data.rigidBodyMass : 1.0f;
        rigidbody.useGravity = data.rigidBodyUseGravity;
        rigidbody.isKinematic =
            string.Equals(data.rigidBodyType, "Static", StringComparison.OrdinalIgnoreCase) ||
            string.Equals(data.rigidBodyType, "Kinematic", StringComparison.OrdinalIgnoreCase) ||
            !data.physicsEnabled;

        ApplyCollider(gameObject, data);
    }

    private static void ApplyCollider(GameObject gameObject, LevelObject data)
    {
        string colliderType = data.rigidBodyCollider ?? string.Empty;

        if (colliderType.Equals("Sphere", StringComparison.OrdinalIgnoreCase))
        {
            if (!gameObject.GetComponent<SphereCollider>()) gameObject.AddComponent<SphereCollider>();
        }
        else if (colliderType.Equals("Capsule", StringComparison.OrdinalIgnoreCase))
        {
            if (!gameObject.GetComponent<CapsuleCollider>()) gameObject.AddComponent<CapsuleCollider>();
        }
        else if (colliderType.Equals("Box", StringComparison.OrdinalIgnoreCase))
        {
            BoxCollider box = gameObject.GetComponent<BoxCollider>();
            if (!box) box = gameObject.AddComponent<BoxCollider>();
            if (data.rigidBodyColliderSize != Vector3.zero) box.size = data.rigidBodyColliderSize;
        }
    }

    private static void ExportLevel(string path)
    {
        LevelFile level = new LevelFile();
        Dictionary<Transform, int> ids = new Dictionary<Transform, int>();

        Transform[] sceneTransforms = Resources.FindObjectsOfTypeAll<Transform>();
        int nextId = 1;
        foreach (Transform transform in sceneTransforms)
        {
            if (!ShouldExport(transform)) continue;
            ids[transform] = nextId++;
        }

        foreach (Transform transform in sceneTransforms)
        {
            if (!ids.TryGetValue(transform, out int id)) continue;

            GameObject gameObject = transform.gameObject;
            Rigidbody rigidbody = gameObject.GetComponent<Rigidbody>();
            Light light = gameObject.GetComponent<Light>();

            LevelObject data = new LevelObject
            {
                id = id,
                parentId = transform.parent && ids.TryGetValue(transform.parent, out int parentId) ? parentId : 0,
                name = gameObject.name,
                type = ResolveType(gameObject),
                texture = string.Empty,
                enabled = gameObject.activeSelf,
                hasRigidBody = rigidbody != null,
                physicsEnabled = rigidbody != null && !rigidbody.isKinematic,
                rigidBodyEnabled = rigidbody != null && !rigidbody.isKinematic,
                rigidBodyType = ResolveRigidBodyType(rigidbody),
                rigidBodyCollider = ResolveColliderType(gameObject),
                rigidBodyColliderSize = ResolveColliderSize(gameObject),
                rigidBodyMass = rigidbody ? rigidbody.mass : 0.0f,
                rigidBodyUseGravity = rigidbody && rigidbody.useGravity,
                pointLightIntensity = light && light.type == LightType.Point ? light.intensity : 0.0f,
                pointLightRange = light && light.type == LightType.Point ? light.range : 0.0f,
                position = transform.localPosition,
                rotation = transform.localEulerAngles,
                scale = transform.localScale
            };

            level.objects.Add(data);
        }

        File.WriteAllText(path, WriteLevel(level));
        AssetDatabase.Refresh();
        Debug.Log($"Exported {level.objects.Count} objects to {path}");
    }

    private static LevelFile ParseLevel(string json)
    {
        LevelFile level = new LevelFile();

        Match objectsMatch = Regex.Match(json, "\"objects\"\\s*:\\s*\\[(.*)\\]", RegexOptions.Singleline);
        if (!objectsMatch.Success) return level;

        foreach (string objectJson in ExtractObjectBlocks(objectsMatch.Groups[1].Value))
        {
            LevelObject data = new LevelObject
            {
                id = ReadInt(objectJson, "id"),
                parentId = ReadInt(objectJson, "parentId"),
                name = ReadString(objectJson, "name"),
                type = ReadString(objectJson, "type"),
                texture = ReadString(objectJson, "texture"),
                enabled = ReadBool(objectJson, "enabled", true),
                hasRigidBody = ReadBool(objectJson, "hasRigidBody") || ReadBool(objectJson, "rigidBody"),
                physicsEnabled = ReadBool(objectJson, "physicsEnabled") || ReadBool(objectJson, "rigidBodyEnabled"),
                rigidBodyEnabled = ReadBool(objectJson, "rigidBodyEnabled") || ReadBool(objectJson, "physicsEnabled"),
                rigidBodyType = ReadString(objectJson, "rigidBodyType"),
                rigidBodyCollider = ReadString(objectJson, "rigidBodyCollider"),
                rigidBodyColliderSize = ReadVector3(objectJson, "rigidBodyColliderSize"),
                rigidBodyMass = ReadFloat(objectJson, "rigidBodyMass"),
                rigidBodyUseGravity = ReadBool(objectJson, "rigidBodyUseGravity"),
                pointLightIntensity = ReadFloat(objectJson, "pointLightIntensity"),
                pointLightRange = ReadFloat(objectJson, "pointLightRange"),
                position = ReadVector3(objectJson, "position"),
                rotation = ReadVector3(objectJson, "rotation"),
                scale = ReadVector3(objectJson, "scale", Vector3.one)
            };

            level.objects.Add(data);
        }

        return level;
    }

    private static List<string> ExtractObjectBlocks(string objectsJson)
    {
        List<string> blocks = new List<string>();
        int depth = 0;
        int start = -1;

        for (int i = 0; i < objectsJson.Length; i++)
        {
            if (objectsJson[i] == '{')
            {
                if (depth == 0) start = i;
                depth++;
            }
            else if (objectsJson[i] == '}')
            {
                depth--;
                if (depth == 0 && start >= 0)
                {
                    blocks.Add(objectsJson.Substring(start, i - start + 1));
                    start = -1;
                }
            }
        }

        return blocks;
    }

    private static string WriteLevel(LevelFile level)
    {
        StringBuilder builder = new StringBuilder();
        builder.AppendLine("{");
        builder.AppendLine("  \"version\": 1,");
        builder.AppendLine("  \"objects\": [");

        for (int i = 0; i < level.objects.Count; i++)
        {
            LevelObject data = level.objects[i];
            builder.AppendLine("    {");
            builder.AppendLine($"      \"id\": {data.id},");
            builder.AppendLine($"      \"parentId\": {data.parentId},");
            builder.AppendLine($"      \"name\": \"{EscapeJson(data.name)}\",");
            builder.AppendLine($"      \"type\": \"{EscapeJson(data.type)}\",");
            builder.AppendLine($"      \"texture\": \"{EscapeJson(data.texture)}\",");
            builder.AppendLine($"      \"enabled\": {WriteBool(data.enabled)},");
            builder.AppendLine($"      \"hasRigidBody\": {WriteBool(data.hasRigidBody)},");
            builder.AppendLine($"      \"physicsEnabled\": {WriteBool(data.physicsEnabled)},");
            builder.AppendLine($"      \"rigidBodyEnabled\": {WriteBool(data.rigidBodyEnabled)},");
            builder.AppendLine($"      \"rigidBodyType\": \"{EscapeJson(data.rigidBodyType)}\",");
            builder.AppendLine($"      \"rigidBodyCollider\": \"{EscapeJson(data.rigidBodyCollider)}\",");
            builder.AppendLine($"      \"rigidBodyColliderSize\": {WriteVector3(data.rigidBodyColliderSize)},");
            builder.AppendLine($"      \"rigidBodyMass\": {FormatFloat(data.rigidBodyMass)},");
            builder.AppendLine($"      \"rigidBodyUseGravity\": {WriteBool(data.rigidBodyUseGravity)},");
            builder.AppendLine($"      \"pointLightIntensity\": {FormatFloat(data.pointLightIntensity)},");
            builder.AppendLine($"      \"pointLightRange\": {FormatFloat(data.pointLightRange)},");
            builder.AppendLine($"      \"position\": {WriteVector3(data.position)},");
            builder.AppendLine($"      \"rotation\": {WriteVector3(data.rotation)},");
            builder.AppendLine($"      \"scale\": {WriteVector3(data.scale)}");
            builder.Append("    }");
            if (i < level.objects.Count - 1) builder.Append(",");
            builder.AppendLine();
        }

        builder.AppendLine("  ]");
        builder.AppendLine("}");
        return builder.ToString();
    }

    private static int ReadInt(string json, string fieldName, int defaultValue = 0)
    {
        Match match = Regex.Match(json, $"\"{fieldName}\"\\s*:\\s*(-?\\d+)");
        return match.Success && int.TryParse(match.Groups[1].Value, out int value) ? value : defaultValue;
    }

    private static float ReadFloat(string json, string fieldName, float defaultValue = 0.0f)
    {
        Match match = Regex.Match(json, $"\"{fieldName}\"\\s*:\\s*(-?\\d+(?:\\.\\d+)?)");
        return match.Success && float.TryParse(match.Groups[1].Value, NumberStyles.Float, CultureInfo.InvariantCulture, out float value) ? value : defaultValue;
    }

    private static bool ReadBool(string json, string fieldName, bool defaultValue = false)
    {
        Match match = Regex.Match(json, $"\"{fieldName}\"\\s*:\\s*(true|false)", RegexOptions.IgnoreCase);
        return match.Success ? string.Equals(match.Groups[1].Value, "true", StringComparison.OrdinalIgnoreCase) : defaultValue;
    }

    private static string ReadString(string json, string fieldName, string defaultValue = "")
    {
        Match match = Regex.Match(json, $"\"{fieldName}\"\\s*:\\s*\"((?:\\\\.|[^\"])*)\"");
        return match.Success ? Regex.Unescape(match.Groups[1].Value) : defaultValue;
    }

    private static Vector3 ReadVector3(string json, string fieldName)
    {
        return ReadVector3(json, fieldName, Vector3.zero);
    }

    private static Vector3 ReadVector3(string json, string fieldName, Vector3 defaultValue)
    {
        Match match = Regex.Match(json, $"\"{fieldName}\"\\s*:\\s*\\[\\s*(-?\\d+(?:\\.\\d+)?)\\s*,\\s*(-?\\d+(?:\\.\\d+)?)\\s*,\\s*(-?\\d+(?:\\.\\d+)?)\\s*\\]");
        if (!match.Success) return defaultValue;

        bool parsedX = float.TryParse(match.Groups[1].Value, NumberStyles.Float, CultureInfo.InvariantCulture, out float x);
        bool parsedY = float.TryParse(match.Groups[2].Value, NumberStyles.Float, CultureInfo.InvariantCulture, out float y);
        bool parsedZ = float.TryParse(match.Groups[3].Value, NumberStyles.Float, CultureInfo.InvariantCulture, out float z);
        return parsedX && parsedY && parsedZ ? new Vector3(x, y, z) : defaultValue;
    }

    private static string WriteVector3(Vector3 value)
    {
        return $"[{FormatFloat(value.x)}, {FormatFloat(value.y)}, {FormatFloat(value.z)}]";
    }

    private static string WriteBool(bool value)
    {
        return value ? "true" : "false";
    }

    private static string FormatFloat(float value)
    {
        return value.ToString("G9", CultureInfo.InvariantCulture);
    }

    private static string EscapeJson(string value)
    {
        return string.IsNullOrEmpty(value)
            ? string.Empty
            : value.Replace("\\", "\\\\").Replace("\"", "\\\"");
    }

    private static bool ShouldExport(Transform transform)
    {
        if (!transform || !transform.gameObject.scene.IsValid()) return false;
        if (transform.hideFlags != HideFlags.None) return false;
        if (transform.GetComponent<Camera>()) return false;
        return true;
    }

    private static string ResolveType(GameObject gameObject)
    {
        Light light = gameObject.GetComponent<Light>();
        if (light && light.type == LightType.Point) return "Point Light";

        MeshFilter meshFilter = gameObject.GetComponent<MeshFilter>();
        string meshName = meshFilter && meshFilter.sharedMesh ? meshFilter.sharedMesh.name.ToLowerInvariant() : string.Empty;

        if (meshName.Contains("plane")) return "Plane";
        if (meshName.Contains("cube")) return "Cube";
        if (meshName.Contains("sphere")) return "Sphere";
        if (meshName.Contains("capsule")) return "Capsule";
        if (meshName.Contains("cylinder")) return "Cylinder";

        return "Empty";
    }

    private static string ResolveRigidBodyType(Rigidbody rigidbody)
    {
        if (!rigidbody) return string.Empty;
        return rigidbody.isKinematic ? "Kinematic" : "Dynamic";
    }

    private static string ResolveColliderType(GameObject gameObject)
    {
        if (gameObject.GetComponent<SphereCollider>()) return "Sphere";
        if (gameObject.GetComponent<CapsuleCollider>()) return "Capsule";
        if (gameObject.GetComponent<BoxCollider>()) return "Box";
        if (gameObject.GetComponent<MeshCollider>()) return "ConcaveMesh";
        return string.Empty;
    }

    private static Vector3 ResolveColliderSize(GameObject gameObject)
    {
        BoxCollider box = gameObject.GetComponent<BoxCollider>();
        if (box) return box.size;

        SphereCollider sphere = gameObject.GetComponent<SphereCollider>();
        if (sphere) return Vector3.one * sphere.radius * 2.0f;

        CapsuleCollider capsule = gameObject.GetComponent<CapsuleCollider>();
        if (capsule) return new Vector3(capsule.radius * 2.0f, capsule.height, capsule.radius * 2.0f);

        return Vector3.zero;
    }
}
#endif
