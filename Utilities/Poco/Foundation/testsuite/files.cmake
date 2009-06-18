SET(CPP_BASE_FILENAMES
 ActiveMethodTest ActivityTest ActiveDispatcherTest
	AutoPtrTest SharedPtrTest AutoReleasePoolTest Base64Test 
	BinaryReaderWriterTest LineEndingConverterTest 
	ByteOrderTest ChannelTest ClassLoaderTest CoreTest CoreTestSuite 
	CountingStreamTest CryptTestSuite DateTimeFormatterTest 
	DateTimeParserTest DateTimeTest LocalDateTimeTest DateTimeTestSuite DigestStreamTest 
	Driver DynamicFactoryTest FPETest FileChannelTest FileTest GlobTest FilesystemTestSuite 
	FoundationTestSuite HMACEngineTest HexBinaryTest LoggerTest 
	LoggingFactoryTest LoggingRegistryTest LoggingTestSuite LogStreamTest 
	NamedEventTest NamedMutexTest ProcessesTestSuite ProcessTest 
	MemoryPoolTest MD2EngineTest MD4EngineTest MD5EngineTest ManifestTest 
	NDCTest NotificationCenterTest NotificationQueueTest 
	NotificationsTestSuite NullStreamTest NumberFormatterTest 
	NumberParserTest PathTest PatternFormatterTest RWLockTest 
	RandomStreamTest RandomTest RegularExpressionTest SHA1EngineTest 
	SemaphoreTest ConditionTest SharedLibraryTest SharedLibraryTestSuite 
	SimpleFileChannelTest StopwatchTest 
	StreamConverterTest StreamCopierTest StreamTokenizerTest 
	StreamsTestSuite StringTest StringTokenizerTest TaskTestSuite TaskTest 
	TaskManagerTest TestChannel TeeStreamTest UTF8StringTest 
	TextConverterTest TextIteratorTest TextTestSuite TextEncodingTest 
	ThreadLocalTest ThreadPoolTest ThreadTest ThreadingTestSuite TimerTest 
	TimespanTest TimestampTest TimezoneTest URIStreamOpenerTest URITest 
	URITestSuite UUIDGeneratorTest UUIDTest UUIDTestSuite ZLibTest 
	TestPlugin DummyDelegate BasicEventTest FIFOEventTest PriorityEventTest EventTestSuite 
	LRUCacheTest ExpireCacheTest ExpireLRUCacheTest CacheTestSuite AnyTest FormatTest 
	HashingTestSuite HashTableTest SimpleHashTableTest LinearHashTableTest 
	HashSetTest HashMapTest SharedMemoryTest PriorityNotificationQueueTest
	UniqueExpireCacheTest UniqueExpireLRUCacheTest TimedNotificationQueueTest
	TuplesTest NamedTuplesTest TypeListTest DynamicAnyTest FileStreamTest
)

SET(CPP_FILES "")
FOREACH(basename ${CPP_BASE_FILENAMES})
  SET(CPP_FILES ${CPP_FILES} src/${basename})
ENDFOREACH(basename ${CPP_BASE_FILENAMES})
