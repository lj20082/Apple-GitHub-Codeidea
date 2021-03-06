/*
 * This file is part of the SDWebImage package.
 * (c) Olivier Poitrey <rs@dailymotion.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#import <Foundation/Foundation.h>
#import "SDWebImageCompat.h"
#import "SDWebImageOperation.h"

//图片下载选项
typedef NS_OPTIONS(NSUInteger, SDWebImageDownloaderOptions) {

    //低优先级
    SDWebImageDownloaderLowPriority = 1 << 0,
    //渐进式下载
    SDWebImageDownloaderProgressiveDownload = 1 << 1,

    /**
     * By default, request prevent the of NSURLCache. With this flag, NSURLCache
     * is used with default policies.
     */
    /*
     默认情况下，请求不使用 NSURLCache。使用此标记，会使用 NSURLCache 和默认缓存策略
     */
    SDWebImageDownloaderUseNSURLCache = 1 << 2,

    /**
     * Call completion block with nil image/imageData if the image was read from NSURLCache
     * (to be combined with `SDWebImageDownloaderUseNSURLCache`).
     */
    
    /*
     * 如果图像是从 NSURLCache 读取的，则调用 completion block 时，image/imageData 传入 nil
     * (此标记要和 `SDWebImageDownloaderUseNSURLCache` 组合使用)
     */
    SDWebImageDownloaderIgnoreCachedResponse = 1 << 3,
    /**
     * In iOS 4+, continue the download of the image if the app goes to background. This is achieved by asking the system for
     * extra time in background to let the request finish. If the background task expires the operation will be cancelled.
     */
    /*
     * 在 iOS 4+，当 App 进入后台后仍然会继续下载图像。这是向系统请求额外的后台时间以保证下载请求完成的
     * 如果后台任务过期，请求将会被取消
     */
    SDWebImageDownloaderContinueInBackground = 1 << 4,

    /**
     * Handles cookies stored in NSHTTPCookieStore by setting 
     * NSMutableURLRequest.HTTPShouldHandleCookies = YES;
     */
    /*
     *  处理保存在 NSHTTPCookieStore 中的 cookies
     */
    SDWebImageDownloaderHandleCookies = 1 << 5,

    /**
     * Enable to allow untrusted SSL certificates.
     * Useful for testing purposes. Use with caution in production.
     */
    /*
     * 允许不信任的 SSL 证书
     * 可以出于测试目的使用，在正式产品中慎用
     */
    SDWebImageDownloaderAllowInvalidSSLCertificates = 1 << 6,

    /**
     * Put the image in the high priority queue.
     */
    /*
     *  将图像放入高优先级队列
     */
    SDWebImageDownloaderHighPriority = 1 << 7,
};

//下载操作的执行方式
typedef NS_ENUM(NSInteger, SDWebImageDownloaderExecutionOrder) {
    /**
     * Default value. All download operations will execute in queue style (first-in-first-out).
     */
    /*
     *  默认值，所有下载操作将按照队列的先进先出方式执行
     */
    SDWebImageDownloaderFIFOExecutionOrder,

    /**
     * All download operations will execute in stack style (last-in-first-out).
     */
    /*
     *  所有下载操作将按照堆栈的后进先出方式执行
     */
    SDWebImageDownloaderLIFOExecutionOrder
};

//使用其他文件中的全局变量
extern NSString *const SDWebImageDownloadStartNotification; //开始下载通知
extern NSString *const SDWebImageDownloadStopNotification;  //停止下载通知

//定义下载进度回调
typedef void(^SDWebImageDownloaderProgressBlock)(NSInteger receivedSize, NSInteger expectedSize);

//定义下载完成回调
typedef void(^SDWebImageDownloaderCompletedBlock)(UIImage *image, NSData *data, NSError *error, BOOL finished);

//定义'头部过滤'回调，有返回值（字典）
typedef NSDictionary *(^SDWebImageDownloaderHeadersFilterBlock)(NSURL *url, NSDictionary *headers);

/**
 * Asynchronous downloader dedicated and optimized for image loading.
 */
//专为加载图像设计并优化的异步下载器
@interface SDWebImageDownloader : NSObject

/**
 * Decompressing images that are downloaded and cached can improve performance but can consume lot of memory.
 * Defaults to YES. Set this to NO if you are experiencing a crash due to excessive memory consumption.
 * 是否解码，如果设置为YES，那么下载和缓存的图像可以提高性能，但会消耗大量的内存。
 * 默认值为YES。如果你需要考虑内存问题，那么请设置为NO
 * SDWebImage框架通过次方式实现 以牺牲内存存储空间来换取性能
 */
@property (assign, nonatomic) BOOL shouldDecompressImages;

//设置并发下载数，默认为6
@property (assign, nonatomic) NSInteger maxConcurrentDownloads;

/**
 * Shows the current amount of downloads that still need to be downloaded
 */
//显示仍需要下载的数量
@property (readonly, nonatomic) NSUInteger currentDownloadCount;


/**
 *  The timeout value (in seconds) for the download operation. Default: 15.0.
 */
//下载操作的超时时长(秒)，默认：15秒
@property (assign, nonatomic) NSTimeInterval downloadTimeout;


/**
 * Changes download operations execution order. Default value is `SDWebImageDownloaderFIFOExecutionOrder`.
 */
//通过该属性，可以修改下载操作执行顺序，默认值是 `SDWebImageDownloaderFIFOExecutionOrder`，即先进先出
@property (assign, nonatomic) SDWebImageDownloaderExecutionOrder executionOrder;

/**
 *  Singleton method, returns the shared instance
 *
 *  @return global shared instance of downloader class
 */
//单例方法，返回一个全局共享的下载器
+ (SDWebImageDownloader *)sharedDownloader;

/**
 *  Set the default URL credential to be set for request operations.
 */
//设置默认的URL身份认证信息
@property (strong, nonatomic) NSURLCredential *urlCredential;

/**
 * Set username
 */
//设置用户名
@property (strong, nonatomic) NSString *username;

/**
 * Set password
 */
//设置密码
@property (strong, nonatomic) NSString *password;

/**
 * Set filter to pick headers for downloading image HTTP request.
 *
 * This block will be invoked for each downloading image request, returned
 * NSDictionary will be used as headers in corresponding HTTP request.
 */
/*
 * 设置下载图像 HTTP 请求头过滤器
 * 此 block 将被每一个下载图像的请求调用，返回的 NSDictionary 将被作为相应的 HTTP 请求头
 */
@property (nonatomic, copy) SDWebImageDownloaderHeadersFilterBlock headersFilter;

/**
 * Set a value for a HTTP header to be appended to each download HTTP request.
 *
 * @param value The value for the header field. Use `nil` value to remove the header.
 * @param field The name of the header field to set.
 */
/*
 * 为 HTTP 请求头设置一个值
 * value 请求头字段的值，使用 `nil` 删除该字段
 * field 要设置的请求头字段名
 */
- (void)setValue:(NSString *)value forHTTPHeaderField:(NSString *)field;

/**
 * Returns the value of the specified HTTP header field.
 *
 * @return The value associated with the header field field, or `nil` if there is no corresponding header field.
 */
/*
 * 返回指定 HTTP 请求头字段的值
 * 返回值为请求头字段的值，如果没有返回 `nil`
 */
- (NSString *)valueForHTTPHeaderField:(NSString *)field;

/**
 * Sets a subclass of `SDWebImageDownloaderOperation` as the default
 * `NSOperation` to be used each time SDWebImage constructs a request
 * operation to download an image.
 *
 * @param operationClass The subclass of `SDWebImageDownloaderOperation` to set 
 *        as default. Passing `nil` will revert to `SDWebImageDownloaderOperation`.
 */
/*
 * 设置 SDWebImage 每次构造下载图像请求操作的 `SDWebImageDownloaderOperation` 的子类
 * 默认操作的 `SDWebImageDownloaderOperation` 的子类，传入 `nil` 将恢复为
 */
- (void)setOperationClass:(Class)operationClass;

/**
 * Creates a SDWebImageDownloader async downloader instance with a given URL
 *
 * The delegate will be informed when the image is finish downloaded or an error has happen.
 *
 * @see SDWebImageDownloaderDelegate
 *
 * @param url            The URL to the image to download
 * @param options        The options to be used for this download
 * @param progressBlock  A block called repeatedly while the image is downloading
 * @param completedBlock A block called once the download is completed.
 *                       If the download succeeded, the image parameter is set, in case of error,
 *                       error parameter is set with the error. The last parameter is always YES
 *                       if SDWebImageDownloaderProgressiveDownload isn't use. With the
 *                       SDWebImageDownloaderProgressiveDownload option, this block is called
 *                       repeatedly with the partial image object and the finished argument set to NO
 *                       before to be called a last time with the full image and finished argument
 *                       set to YES. In case of error, the finished argument is always YES.
 *
 * @return A cancellable SDWebImageOperation
 */
/*
 * 使用给定的 URL 创建 SDWebImageDownloader 异步下载器实例
 * 图像下载完成或者出现错误时会通知代理
 * url:要下载的图像 URL
 * SDWebImageDownloaderOptions：下载选项|策略
 * progressBlock：图像下载过程中被重复调用的 block，用来报告下载进度
 * completedBlock：图像下载完成后被调用一次的 block
 *      image:如果下载成功，image 参数会被设置
 *      error:如果出现错误，error 参数会被设置
 *      finished:
            如果没有使用 SDWebImageDownloaderProgressiveDownload，最后一个参数一直是 YES
 *          如果使用了 SDWebImageDownloaderProgressiveDownload 选项，此 block 会被重复调用
 *              1)下载完成前，image 参数是部分图像，finished 参数是 NO
 *              2)最后一次被调用时，image 参数是完整图像，而 finished 参数是 YES
 *              3)如果出现错误，那么finished 参数也是 YES
 *  返回值：可被取消的 SDWebImageOperation
 */
- (id <SDWebImageOperation>)downloadImageWithURL:(NSURL *)url
                                         options:(SDWebImageDownloaderOptions)options
                                        progress:(SDWebImageDownloaderProgressBlock)progressBlock
                                       completed:(SDWebImageDownloaderCompletedBlock)completedBlock;

/**
 * Sets the download queue suspension state
 */
/*
 * 设置下载队列挂起状态
 */
- (void)setSuspended:(BOOL)suspended;

@end
