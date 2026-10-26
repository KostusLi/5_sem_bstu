using System.Text.Json;

namespace ResultsCollection
{
    public class Results
    {
        private readonly string _filePath;

        private readonly object _fileLock = new object();

        public Results()
        {
            _filePath = "RESULTS.json";
        }

        private Dictionary<int, string> LoadData()
        {
            if (!File.Exists(_filePath))
            {
                return new Dictionary<int, string>();
            }

            var json = File.ReadAllText(_filePath);
            return JsonSerializer.Deserialize<Dictionary<int, string>>(json) ?? new Dictionary<int, string>();
        }

        private void SaveData(Dictionary<int, string> data)
        {
            var json = JsonSerializer.Serialize(data, new JsonSerializerOptions { WriteIndented = true });
            File.WriteAllText(_filePath, json);
        }

        public Dictionary<int, string> GetAll()
        {
            lock (_fileLock)
            {
                return LoadData();
            }
        }

        public string? GetById(int key)
        {
            lock (_fileLock)
            {
                var data = LoadData();
                return data.TryGetValue(key, out var value) ? value : null;
            }
        }

        public KeyValuePair<int, string> Add(string value)
        {
            lock (_fileLock)
            {
                var data = LoadData();

                int newKey = data.Keys.Count > 0 ? data.Keys.Max() + 1 : 1;

                data.Add(newKey, value);
                SaveData(data);

                return new KeyValuePair<int, string>(newKey, value);
            }
        }

        public bool Change(int key, string value)
        {
            lock (_fileLock)
            {
                var data = LoadData();
                if (!data.ContainsKey(key))
                {
                    return false;
                }

                data[key] = value;
                SaveData(data);
                return true;
            }
        }

        public bool DeleteById(int key)
        {
            lock (_fileLock)
            {
                var data = LoadData();
                if (!data.Remove(key))
                {
                    return false;
                }

                SaveData(data);
                return true;
            }
        }
    }
}