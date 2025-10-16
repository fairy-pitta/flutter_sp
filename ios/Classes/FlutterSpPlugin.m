#import "FlutterSpPlugin.h"

@implementation FlutterSpPlugin

+ (void)registerWithRegistrar:(NSObject<FlutterPluginRegistrar>*)registrar {
  // Plugin registration - FFI handles the native calls directly
  // This is a placeholder for any iOS-specific setup if needed
}

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result {
  // Method calls are handled through FFI, not platform channels
  result(FlutterMethodNotImplemented);
}

@end